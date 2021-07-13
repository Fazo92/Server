#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "sock.h"




//UDP Socket erstellen. Portnummer und IP-Adresse angeben. 
void sock::initUDP(int port, string clientIP) {
	WSADATA data;

	// To start WinSock, the required version must be passed to
	// WSAStartup(). This server is going to use WinSock version
	// 2 so I create a word that will store 2 and 2 in hex i.e.
	// 0x0202
	WORD version = MAKEWORD(2, 2);

	// Start WinSock
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		// Not ok! Get out quickly
		cout << "Can't start Winsock! " << wsOk;
		return;
	}
	SOCKET sockettmp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in serverHint_send, serverHint_recv;
	serverHint_send.sin_family = AF_INET; // Address format is IPv4
	serverHint_send.sin_port = htons(port); // Convert from little to big endian

	serverHint_recv.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint_recv.sin_family = AF_INET; // Address format is IPv4
	serverHint_recv.sin_port = htons(port); // Convert from little to big endian

	inet_pton(AF_INET, clientIP.c_str(), &serverHint_send.sin_addr);

	// Try and bind the socket to the IP and port
	int bin = ::bind(sockettmp, (sockaddr*)&serverHint_recv, sizeof(serverHint_recv));
	if (bin == -1)
	{
		std::cout << "Can't bind socket! " << WSAGetLastError() << endl;

	}
	//Abspeichern in membervariablen
	this->in = sockettmp;
	this->server = serverHint_send;
	this->serverLength = sizeof(server);
}


//Erstellen eines Multicast-UDPSockets
void sock::initMultiCastUDP(int port, string localIP, string groupIP)
{
	int sockfd;
	struct in_addr local;
	struct sockaddr_in group;

	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0) {
		cout << "WSA FAILED\n";
	}

	//Prüfen ob Socket erstellt werden kann
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("error on sockfd \n");
	}
	else {
		printf("send socket created \n");
	}

	memset((char*)&group, 0, sizeof(group));
	group.sin_family = AF_INET; //IPv4
	group.sin_addr.s_addr = inet_addr(groupIP.c_str()); //Gruppen IP-Adresse angeben
	group.sin_port = htons(port); //Port angeben

	local.s_addr = inet_addr(localIP.c_str()); //localIP ist die IP-Adresse des Computers
	if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&local, sizeof(local)) == -1) {
		cout << "can't create multicast Socket" << endl;
	}
	else {
		printf("local interface set");
	}

	this->in = sockfd;
	this->server = group;
	this->serverLength = sizeof(group);
}


//Erstellen eines Multicast-Sockets, falls Daten erhalten werden sollen
void sock::initMultiCastUDPrecv(int port, string localIP, string groupIP)
{

	//CODE on a window system
	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0) {
		cout << "WSA FAILED\n";
	}

	int sockfd;
	struct sockaddr_in local;
	struct ip_mreq group;
	char databuf[1024];
	int datalen;
	struct sockaddr_in src_addr;    /* Used to receive (addr,port) of sender */


	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("error creating socket \n");
	}
	else {
		printf("socket created \n");
	}

	int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) == 1) {
		perror("error on setsockopt \n");
	}
	else {
		printf("sockopt'd successfully \n");
	}

	memset((char*)&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = INADDR_ANY;

	if (::bind(sockfd, (struct sockaddr*)&local, sizeof(local)) == -1) {
		perror("error on binding \n");
	}
	else {
		printf("bound \n");
	}

	group.imr_multiaddr.s_addr = inet_addr(groupIP.c_str());
	group.imr_interface.s_addr = inet_addr(localIP.c_str());
	if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char*)&group, sizeof(group)) == -1) {
		perror("adding group error \n");
		closesocket(sockfd);
		exit(1);
	}
	else {
		printf("adding multicast group \n");
	}
	this->in = sockfd;
}


//Funktion, um TCP Socket zu erstellen
void sock::initTCP(int port)
{
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket" << std::endl;

	}

	sockaddr_in hint;
	//hint.sin_addr.s_addr = inet_addr("127.0.0.1");
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	//Binden des Sockets an IP-Adresse
	::bind(listening, (sockaddr*)&hint, sizeof(hint));
	listen(listening, SOMAXCONN);

	//Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	char host[NI_MAXHOST];
	char service[NI_MAXHOST];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << "connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
	}
	in = clientSocket;
	closesocket(listening);
	//tcp ist hier eine boolsche membervariable, die auf true gesetzt wird
	tcp = true;
}

void sock::send_img(cv::Mat img) {
	vector <uchar> imgVec;
	vector < int > compression_params;
	compression_params.push_back(IMWRITE_JPEG_QUALITY);
	compression_params.push_back(80);
	sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));

	imencode(".jpg", img, imgVec, compression_params);
	if (tcp == true) {
		int vecSize = imgVec.size();
		send(in, (char*)&vecSize, sizeof(int), 0);
		send(in, (char*)&imgVec[0], imgVec.size(), 0);

	}
	else
	{
		int total_pack = 1 + (imgVec.size() - 1) / PACK_SIZE;
		int ibuf[1];
		ibuf[0] = total_pack;
		int s = sendto(in, (char*)&ibuf, sizeof(int), 0, (sockaddr*)&server, sizeof(server));
		//int s = sendto(in, st.c_str(), sizeof(st), 0, (sockaddr*)&server, sizeof(server));
		if (s == -1)
		{
			cout << "That didn't work! " << WSAGetLastError() << endl;
		}
		for (int i = 0; i < total_pack; i++)
			sendto(in, (char*)&imgVec[i * PACK_SIZE], PACK_SIZE, 0, (sockaddr*)&server, sizeof(server));
	}
}

void sock::rcv_img(Mat& img)
{
	int recvMsgSize; // Size of received message

	//Wenn TCP-Socket erstellt wurde dann tcp=true
	if (tcp == true) {
		int bytesDim = 0;
		int bytes = 0;
		char buf2[4];
		int imgSize;
		ZeroMemory(buf2, sizeof(buf2));

		//Wait for client to send data

		if ((bytesDim = recv(in, buf2, sizeof(buf2), 0)) == -1) cout << "recv number failed" << endl;
		int* pt = (int*)buf2;
		imgSize = *pt;
		char* buf = new char[imgSize];
		ZeroMemory(buf, imgSize);

		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(in, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		Mat rawData = Mat(1, imgSize, CV_8UC1, buf);
		Mat frame = imdecode(rawData, IMREAD_COLOR);
		delete[] buf;

	}
	else {

		char buffer[65540]; // Buffer for echo string

		int l = getClientLength();
		sockaddr_in cl = getsockaddr_in();
		sockaddr_in cl2;
		int l2 = sizeof(cl2);
		// recvMsgSize gibt die Anzahl an Bytes an, die empfangen wird
		do {

			recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2); //Erhalten von Anzahl der Pakete vom Client
		} while (recvMsgSize > sizeof(int));


		long int total_pack = ((int*)buffer)[0]; //total_pack gibt Anzahl der Pakete an

		//Puffer erstellen der genügend Bytes beinhaltet, um das Bild abzuspeichern

		char* longbuf = new char[PACK_SIZE * total_pack];

		//For schleife in der kodierte Daten des zu empfangenen Bildes erhalten werden und in buffer temporär gespeichert werden
		for (int i = 0; i < total_pack; i++) {
			recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2);

			memcpy(&longbuf[i * PACK_SIZE], buffer, PACK_SIZE); //Kopieren der empfangen Daten in Variable longbuf an der Stelle i*PACK_SIZE
		}

		//Speichern des Bildes 
		Mat rawData = Mat(1, PACK_SIZE * total_pack, 0, longbuf);
		//Dekodieren des BIldes
		Mat frame = imdecode(rawData, IMREAD_COLOR);
		//In Referenzvariable speichern
		img = frame.clone();

		//if (frame.size().width == 0) {
		//	cerr << "decode failure!" << endl;
		//	continue;
		//}

		//Löschen der Variable auf dem Heap, um reservierten Arbeitsspeicher freizugeben
		delete[] longbuf;

	}
}

//Senden einer Boolschen Variable
void sock::send_hindernis(bool hindernis) {

	if (tcp == true) {
		int sendedBytes=send(in, (char*)&hindernis, sizeof(bool), 0);
		if (sendedBytes == -1) {
			cout << "Can't send bool" <<WSAGetLastError()<< endl;
		}
	}
	else {
		int sendedBytes = sendto(in, (char*)&hindernis, sizeof(bool), 0, (sockaddr*)&server, sizeof(server));
		if (sendedBytes == -1) {
			cout << "Can't send bool" << WSAGetLastError()<< endl;
		}
	}
}

int sock::getClientLength() {
	return this->serverLength;

}

void sock::closeSock() {
	closesocket(this->in);
	WSACleanup();
}

sockaddr_in sock::getsockaddr_in() {
	return server;

}


