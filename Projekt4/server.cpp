#include "server.h"

//server::server() {
//}

SOCKET server::createUDPSocket(int portNumber) 
{
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int ws0k = WSAStartup(version, &data);
	if (ws0k != 0) {
		cout << "Can't start Winsock! " << ws0k;
	}
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(portNumber);
	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket!" << WSAGetLastError() << endl;
		return 1;
	}else{
		cout << "Erfolgreich Verbunden UDP" << endl;
	}

	return in;
	
	//WSACleanup();
}