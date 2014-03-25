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

BOOL TCP_Client::Connect(string server, int port)
{
	struct sockaddr_in RemoteHost;

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
		s = socket(AF_INET, SOCK_STREAM, 0);
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

	//Connect to remote server
	if (connect(s, (struct sockaddr *)&RemoteHost, sizeof(RemoteHost)) < 0)
	{
		DEBUG(L"connect() failed");
		return FALSE;
	}

	connected = TRUE;
	starttime = time(0);
	stats.download = 0;
	stats.upload = 0;
	return TRUE;
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