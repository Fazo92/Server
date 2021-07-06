#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "sock.h"





void sock::rcv_depth_img(Mat& img)
{
	//while (true) {
	int recvMsgSize; // Size of received message
	char buflen[sizeof(int)];
	if (tcp == true) {
		int bytesDim = 0;
		int bytes = 0;
		char buf2[4];
		int imgSize;
		ZeroMemory(buf2, sizeof(buf2));

		//Wait for client to send data
		//for (int i = 0; i < sizeof(buf2); i += bytesDim)
		if ((bytesDim = recv(in, buf2, sizeof(buf2), 0)) == -1) cout << "recv number failed" << endl;
		int* pt = (int*)buf2;
		imgSize = *pt;
		char* buf = new char[imgSize];
		ZeroMemory(buf, imgSize);

		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(in, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		Mat rawData = Mat(1, imgSize, CV_64F, buf);
		Mat frame = imdecode(rawData, IMREAD_ANYDEPTH);
		img = frame.clone();
		delete[] buf;
		//imshow("frame", frame);
		//waitKey(1);

	}
	else {

		char buffer[65540]; // Buffer for echo string

		int l = getClientLength();
		sockaddr_in cl = getsockaddr_in();
		sockaddr_in cl2;
		int l2 = sizeof(cl2);
		//ZeroMemory(&cl, sock1.getClientLength()); // Clear the client structure
		//ZeroMemory(buffer, 65540); // Clear the receive buffer
		do {

			recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2);
		} while (recvMsgSize > sizeof(int));
		long int total_pack = ((int*)buffer)[0];

		char* longbuf = new char[PACK_SIZE * total_pack];
		for (int i = 0; i < total_pack; i++) {
			recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2);
			if (recvMsgSize != PACK_SIZE) {
				//cerr << "Received unexpected size pack:" << recvMsgSize << endl;
				continue;
			}
			memcpy(&longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
		}
		Mat rawData = Mat(1, PACK_SIZE * total_pack, CV_64F, longbuf);
		Mat frame = imdecode(rawData, IMREAD_ANYDEPTH);
		img = frame.clone();
		//if (frame.size().width == 0) {
		//	cerr << "decode failure!" << endl;
		//	continue;
		//}
		//namedWindow("recv", WINDOW_FREERATIO);
		//imshow("recv", frame);
		delete[] longbuf;

		//waitKey(0);
	}
	//}
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
	//serverHint.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint_send.sin_family = AF_INET; // Address format is IPv4
	serverHint_send.sin_port = htons(port); // Convert from little to big endian

	serverHint_recv.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint_recv.sin_family = AF_INET; // Address format is IPv4
	serverHint_recv.sin_port = htons(port); // Convert from little to big endian

	inet_pton(AF_INET, clientIP.c_str(), &serverHint_send.sin_addr);

	//// Try and bind the socket to the IP and port
	int bin = ::bind(sockettmp, (sockaddr*)&serverHint_recv, sizeof(serverHint_recv));
	if (bin == -1)
	{
		std::cout << "Can't bind socket! " << WSAGetLastError() << endl;

	}
	this->in = sockettmp;
	this->server = serverHint_send;
	this->serverLength = sizeof(server);
}
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

	::bind(listening, (sockaddr*)&hint, sizeof(hint));
	listen(listening, SOMAXCONN);

	//Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	//SOCKET clientSocket = accept(listening, NULL, NULL);

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
	tcp = true;
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

void sock::rcv_img(Mat& img)
{
		int recvMsgSize; // Size of received message
		char buflen[sizeof(int)];
		if (tcp == true) {
			int bytesDim = 0;
			int bytes = 0;
			char buf2[4];
			int imgSize;
			ZeroMemory(buf2, sizeof(buf2));

			//Wait for client to send data
			//for (int i = 0; i < sizeof(buf2); i += bytesDim)
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
			//ZeroMemory(&cl, sock1.getClientLength()); // Clear the client structure
			//ZeroMemory(buffer, 65540); // Clear the receive buffer
			do {

				recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2);
			} while (recvMsgSize > sizeof(int));
			long int total_pack = ((int*)buffer)[0];

			char* longbuf = new char[PACK_SIZE * total_pack];
			for (int i = 0; i < total_pack; i++) {
				recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2);
				//if (recvMsgSize != PACK_SIZE) {
				//	cerr << "Received unexpected size pack:" << recvMsgSize << endl;
				//	continue;
				//}
				memcpy(&longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
			}


			Mat rawData = Mat(1, PACK_SIZE * total_pack, 0, longbuf);
			Mat frame = imdecode(rawData, IMREAD_COLOR);
			img = frame.clone();
			//if (frame.size().width == 0) {
			//	cerr << "decode failure!" << endl;
			//	continue;
			//}

			delete[] longbuf;

	}
}






void sock::initMultiCastUDPrecv(int port, string localIP, string groupIP)
{

	//CODE because I'm creating on a window system
	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0) {
		cout << "WSA FAILED\n";
	}
	//CODE because I'm creating on a window system
	//The rest	of	the code	would	suffice alone on Linux

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

void sock::initUDPSend(int port, string clientIP) {

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
	serverHint_send.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint_send.sin_family = AF_INET; // Address format is IPv4
	serverHint_send.sin_port = htons(port); // Convert from little to big endian

	inet_pton(AF_INET, clientIP.c_str(), &serverHint_send.sin_addr);

	this->in = sockettmp;
	this->server = serverHint_send;
	this->serverLength = sizeof(server);
}

void sock::initMultiCastUDP(int port, string localIP, string groupIP)
{
	int sockfd;
	struct in_addr local;
	struct sockaddr_in group;

	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0) {
		cout << "WSA FAILED\n";
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("error on sockfd \n");
	}
	else {
		printf("send socket created \n");
	}

	memset((char*)&group, 0, sizeof(group));
	group.sin_family = AF_INET;
	group.sin_addr.s_addr = inet_addr(groupIP.c_str());
	group.sin_port = htons(port);

	local.s_addr = inet_addr(localIP.c_str());
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