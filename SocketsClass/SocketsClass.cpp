#include "SocketsClass.h"

void XTrace(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	TCHAR szBuffer[512]; // get rid of this hard-coded buffer
	nBuf = _vsnwprintf_s(szBuffer, 511, lpszFormat, args);
	::OutputDebugString(szBuffer);
	va_end(args);
}


TCP_Client::TCP_Client(TCP_Client_opts opts_set)
{
	SetOptions(opts_set);
}

TCP_Client::~TCP_Client()
{
	Disconnect();
}

TCP_Client::TCP_Client()
{
}

void TCP_Client::SetOptions(TCP_Client_opts opts_set)
{
	opts = opts_set;
}

TCP_Client_opts TCP_Client::GetOptions()
{
	return opts;
}

SOCKET TCP_Client::GetSocket()
{
	return s;
}

BOOL TCP_Client::IsConnected()
{
	return connected;
}

int TCP_Client::Disconnect()
{
	connected = FALSE;
	stats.uptime = time(0) - starttime;
	return shutdown(s, 2);
}

BOOL TCP_Client::Connect(string server, int port, int timeout)
{
	struct sockaddr_in RemoteHost;
	TIMEVAL Timeout;
	Timeout.tv_sec = timeout;
	Timeout.tv_usec = 0;

#ifdef W32
	WSADATA       wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		DEBUG(L"Failed to load Winsock!\n");
		return FALSE;
	}
#endif

	//create socket if it is not already created
	if (s == SOCKET_ERROR)
	{
		//Create socket
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == SOCKET_ERROR)
		{
			DEBUG(L"Could not create socket");
			return FALSE;
		}
	}

	//setup address structure
	if (inet_addr(server.c_str()) == INADDR_NONE)
	{
		struct hostent *he;

		//resolve the hostname, its not an ip address
		if ((he = gethostbyname(server.c_str())) == NULL)
		{
			//gethostbyname failed
			DEBUG(L"gethostbyname() - Failed to resolve hostname\n");
			return FALSE;
		}
	}	
	else//plain ip address
	{
		RemoteHost.sin_addr.s_addr = inet_addr(server.c_str());
	}

	RemoteHost.sin_family = AF_INET;
	RemoteHost.sin_port = htons(port);

	//set the socket in non-blocking
	unsigned long iMode = 1;
	int iResult = ioctlsocket(s, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
	{
		DEBUGP(L"ioctlsocket failed with error: %ld\n", iResult);
		return FALSE;
	}

	//Connect to remote server
	if (connect(s, (struct sockaddr *)&RemoteHost, sizeof(RemoteHost)) < 0)
	{
		printf("%i", WSAGetLastError());
#ifdef W32
		if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
		if (errno != EINPROGRESS)
#endif
		{
			DEBUG(L"connect() failed");
			return FALSE;
		}		
	}

	// restart the socket mode
	iMode = 0;
	iResult = ioctlsocket(s, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
	{
		DEBUGP(L"ioctlsocket failed with error: %ld\n", iResult);
		return FALSE;
	}

	fd_set Write, Err;
	FD_ZERO(&Write);
	FD_ZERO(&Err);
	FD_SET(s, &Write);
	FD_SET(s, &Err);

	// check if the socket is ready
	select(0, NULL, &Write, &Err, &Timeout);
	if (FD_ISSET(s, &Write))
	{
		connected = TRUE;
		starttime = time(0);
		stats.download = 0;
		stats.upload = 0;
		return TRUE;
	}
	return FALSE;
}

TCP_Stats TCP_Client::GetStats()
{
	if (connected==TRUE)
		stats.uptime = time(0)-starttime;
	return stats;
}

int TCP_Client::Send(string data)
{
	stats.upload += data.length();
	return send(s, data.c_str(), data.length(), 0);
	
}

int TCP_Client::Recv(string *data)
{
	int ret = 0;
	char buffer[512];

	ret = recv(s, buffer, sizeof(buffer), 0);
	data->assign(buffer);
	data->resize(ret);
	stats.download += data->length();
	
	return ret;
}