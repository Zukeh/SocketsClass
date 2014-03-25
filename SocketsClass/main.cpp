#include <stdio.h>
#include <string.h>
#include "SocketsClass.h"

using namespace std;


int main(int argc, char *argv)
{
	TCP_Client tc;
	tc.Connect("127.0.0.1", 9999);
	tc.Send("HEllo");
	string data;
	tc.Recv(&data);
	puts(data.c_str());
	tc.Disconnect();
	printf("\n\nDL: %i\nUP: %i\nUptime: %u\n", tc.GetStats().download, tc.GetStats().upload, tc.GetStats().uptime);
	return 0;
}