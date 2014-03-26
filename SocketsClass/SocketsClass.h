#ifndef _SOCKETSCLASS_H
#define _SOCKETSCLASS_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	#define W32
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>

	#define SOCKET int
#endif
#include <string>
#include<ctime>
#include <stdio.h>
#include <stdarg.h>
#include <varargs.h>
#include <tchar.h>

using namespace std;

#ifdef _DEBUG
	#define DEBUG(msg) XTrace(msg)
	#define DEBUGP(msg, p) XTrace(msg, p)
#else
	#define DEBUG(msg, params)
#endif

struct TCP_Client_opts
{
	BOOL	UseSCprotocol;
	BOOL	UseEncryption;
	BOOL	UseCompression;
	int		CompressionLevel;
	void	*Callback;
	BOOL	async;
};

struct TCP_Stats
{
	unsigned long int	upload; //bytes
	unsigned long int	download;//bytes
	time_t				uptime; //seconds
};

class TCP_Client
{
	public:
		TCP_Client();
		TCP_Client(TCP_Client_opts opts_set);
		~TCP_Client();
		SOCKET			GetSocket();
		void			SetOptions(TCP_Client_opts opts_set);
		TCP_Client_opts	GetOptions();
		BOOL			Connect(string server, int port, int timeout = 30);
		int				Send(string data);
		int				Recv(string *data);
		BOOL			IsConnected();
		int				Disconnect();
		TCP_Stats		GetStats();
	private:
		SOCKET			s = SOCKET_ERROR;
		TCP_Client_opts opts;
		TCP_Stats		stats;
		BOOL			connected = FALSE;
		time_t			starttime;
};











#endif