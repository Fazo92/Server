#pragma once
#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


#define PACK_SIZE 65500  //Paketgröße eines UDP-Datagramms. Eventuell verringern, falls das Bild nicht richtig ankommt
#define ENCODE_QUALITY 80 // Komprimierungsgrad beim Kodieren des Bildes

using namespace std;
using namespace cv;

class sock {
public:
	int serverLength;
	int getClientLength();
	bool tcp = false;

	sockaddr_in server;
	sockaddr_in getsockaddr_in();
	SOCKET in; 

	void initUDP(int port, string clientIP);
	void initMultiCastUDP(int port, string localIP, string groupIP);
	void initMultiCastUDPrecv(int port, string localIP, string groupIP);
	void initTCP(int port);
	void closeSock();

	void send_img(cv::Mat img);
	void rcv_img(Mat& img);
	void send_hindernis(bool hindernis);


};