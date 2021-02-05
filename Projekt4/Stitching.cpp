#include "Stitching.h"
std::mutex mu;
std::mutex mu2;
std::mutex mu3;
std::mutex mu4;
#include <fstream>
//
//Stitching::Stitching(String adress1, String adress2, String adress3 = 0, String adress4 = 0) {
//	//Mat imgLeft = imread(adress1, IMREAD_COLOR);
//	//Mat imgRight = imread(adress2, IMREAD_COLOR);
//	//Mat imgCenter = imread(adress3, IMREAD_COLOR);
//	//Mat imgBottom1 = imread(adress4, IMREAD_COLOR);
//
//	this->imgLeft = imgLeft;
//	this->imgRight = imgRight;
//	this->imgCenter = imgCenter;
//	this->imgBottom1 = imgBottom1;
//	//Mat grayLimage, grayRimage, grayCenter, grayBottom1, grayBottom2;
//	//vector <Mat> imgSum;
//	//Size size(imgLeft.cols + imgRight.cols, imgLeft.rows);
//	flip(imgBottom1, imgBottom1, -1);
//	//resize(imgLeft, imgLeft, size);
//	//resize(imgRight, imgRight, size);
//	//resize(imgCenter, imgCenter, size);
//	//resize(imgBottom1, imgBottom1, size);
//	cvtColor(imgLeft, imgLeft, COLOR_RGB2GRAY);
//	cvtColor(imgRight, imgRight, COLOR_RGB2GRAY);
//	cvtColor(imgCenter, imgCenter, COLOR_RGB2GRAY);
//	cvtColor(imgBottom1, imgBottom1, COLOR_RGB2GRAY);
//	//this->imgLeft = imgLeft;
//	//this->imgRight = imgRight;
//	//this->imgCenter = imgCenter;
//	//this->imgBottom1 = imgBottom1;
//
//}



Stitching::Stitching() {
	Mat imgLeft;
	Mat imgRight;
	Mat imgCenter;
	Mat imgBottom1;
	Mat imgBottom2;

	this->imgLeft = imgLeft;
	this->imgRight = imgRight;
	this->imgCenter = imgCenter;
	this->imgBottom1 = imgBottom1;
	this->imgBottom2 = imgBottom2;

}

void Stitching::realTimeStitching()
{
	//std::this_thread::sleep_for(20s);
	Mat matchesimg1;
	Mat dscCenter, dscLeft;
	vector<KeyPoint> kpCenter, kpLeft;
	while (true) {
		kpCenter = this->m_kpC;
		kpLeft= this->m_kpL;
		dscCenter = this->dscCenter;
		dscLeft = this->dscLeft;
		if (dscCenter.empty() == true || dscLeft.empty() ==true) {
			continue;
		}

		Ptr<cv::DescriptorMatcher> matcher11;
		matcher11 = BFMatcher::create(NORM_L2, false);
		vector<vector<DMatch>> matchesknn;
		matcher11->knnMatch(dscCenter, dscLeft, matchesknn, 2);
		cout << "Angekommen" << endl;
		vector<DMatch> mt1;
		if (matchesknn.size() > 10) {
			for (int i = 0; i < matchesknn.size(); i++) {
				if (matchesknn[i][0].distance < 0.75 * matchesknn[i][1].distance) {
					mt1.push_back(matchesknn[i][0]);
				}
			}
			//setKeypoints(this->imgCenter, kpCenter);
			//setKeypoints(this->imgLeft, kpLeft);
		}
		else {
			continue;
		}
		Mat hCL = getHomography(this->imgCenter, this->imgLeft, mt1);
	}

}

SOCKET createSocket(int port) {
	int bytes = 0;
	int bytes1 = 0;
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int ws0k = WSAStartup(ver, &wsData);
	if (ws0k != 0) {
		cerr << "Can't initialize winsock" << endl;
	}
	//Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket" << endl;

	}

	//Bind the op adress and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
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
		cout << host << "connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}

	closesocket(listening);
	return clientSocket;
}

void Stitching::getDimensions(int port) {
	std::this_thread::sleep_for(0.1s);
	SOCKET clientSocket = createSocket(port);
	char buf[sizeof(uint)];
	int bytes = 0;
	int buffer;
	while (true)
	{
		ZeroMemory(buf, 8);
		//for (int i = 0; i < 8; i += bytes)
		if ((bytes = recv(clientSocket, buf, 8, 0)) == -1) cout << ("recv failed")<<endl;
		this->kpLength = (uint)buf;
		cout << "KP SIZE: " << this->kpLength << endl;
		//Wait for client to send data
		//for (int i = 0; i < 4; i += bytes)
		//if ((bytes = recv(clientSocket, (char*)&buffer, 4+i, 0)) == -1) cout << ("recv failed")<<endl;
		//if (port == 51000) {
		//	this->rowdscCenter = buffer;
		//}
		//else if (port == 61000) {
		//	this->rowdscLeft = buffer;
		//}

		//for (int i = 0; i < 4; i += bytes)
		//if ((bytes = recv(clientSocket, (char*)&buffer, 4, 0)) == -1) cout << ("recv failed")<<endl;
		//if (port == 51000) {
		//	this->coldscCenter = buffer;
		//}
		//else if (port == 61000) {
		//	this->coldscLeft = buffer;
		//}

	}
	closesocket(clientSocket);

	WSACleanup();
}


void Stitching::getCUDADimensions(int port) {
	std::this_thread::sleep_for(0.1s);
	SOCKET clientSocket = createSocket(port);
	char buf[16];
	int bytes = 0;
	int buffer;
	int bufKP;
	while (true)
	{
		ZeroMemory((char*)&buffer, 4);
		ZeroMemory((char*)&bufKP, 4);

		//Wait for client to send data
		for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&bufKP + i, 4 - i, 0)) == -1) cout << ("recv failed");
		mu.lock();
		this->kpLength = bufKP;
		mu.unlock();

		for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv failed");
		this->dscSize = buffer;
	}
	closesocket(clientSocket);

	WSACleanup();
}

void Stitching::getDescriptorTCP(int port)
{
	std::this_thread::sleep_for(2s);
	SOCKET clientSocket = createSocket(port);
	//SOCKET clientSocket = this->serv.createUDPSocket(52000);

	int bytes = 0;
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);
	int buffsize = 1024 * 1024 ;
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	//float buf[30720];
	//float buf[90720];
	float buf[1024 * 1024 ];
	char buffer[1024 * 1024];
	int dscSize;
	while (true)
	{
		vector<float> descriptors;
		ZeroMemory(buffer, sizeof(buffer));
		//Wait for client to send data

		//for (int i = 0; i < imgSize; i += bytes)
		if ((bytes = recv(clientSocket, buffer, sizeof(buffer), 0)) == -1) cout << ("recv failed");
		float *p;

		for (p = (float*)&buffer[0]; p <= (float*)&buffer[bytes-1]; p++) {
			descriptors.push_back(*p);
			//*p++;

		}
		*p = NULL;

		if (bytes == SOCKET_ERROR) {
			cout << "Error receiving from client" << WSAGetLastError() << endl;
			continue;
		}

		
		if (port == 60000)
		{

		}
		else if (port == 52000) {

		}



		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}

		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}




	}
	closesocket(clientSocket);

	WSACleanup();
}
void Stitching::getCudaDescriptorsTCP(int port)
{
	//std::this_thread::sleep_for(2s);

	SOCKET clientSocket = createSocket(port);
	int rows=400, cols=64;
	if (port == 52000) {

	}
	else if (port == 6000) {

	}

	int bytes = 0;
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);
	int buffsize = 1024 * 1024;
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	//float buf[30720];
	//float buf[90720];
	float buf[1024 * 1024];
	//char buf[1024 * 1024];
	vector<float> descriptors;
	int size;
	while (true)
	{
		bytes = 0;
		size = this->dscSize;
		descriptors.clear();
		ZeroMemory((char*)&buf, size);

		for (int i = 0; i < size; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buf+i, size -i, 0)) == -1) cout << ("recv failed dsc");
			
		//cout << "DSC: " << *buf << endl;
		int ptr = 0;
		float *p = buf;
		mu4.lock();
			for (int i = 0; i < size; i++) {

				descriptors.push_back(*p++);
		}
			mu4.unlock();
		
			if (bytes == SOCKET_ERROR) {
			cout << "Error receiving from client" << WSAGetLastError() << endl;
			continue;
		}

		if(port==60000)
		{
		}
		else if (port == 52000) {
		}



		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}

		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}



	}
	closesocket(clientSocket);

	WSACleanup();
}



void Stitching::getKeyPointsTCP(int port) {
	std::this_thread::sleep_for(2s);
	SOCKET clientSocket = createSocket(port);
	int bytes = 0;
	vector<KeyPoint> veckp;
	KeyPoint kp;
	
	char buf[1024*1024];
	//KeyPoint buffer[3000];
	float buffer;
	KeyPoint *p;
	Mat imgK;
	int len = 800000;
	while (true)
	{	
		vector<KeyPoint> veckp;
		//if (port == 59000) {
		//	mu2.lock();
		//	this->m_kpL.clear();
		//	mu2.unlock();
		//}
		//else if (port == 53000) {

		//	mu.lock();
		//	this->m_kpC.clear();
		//	mu.unlock();
		//}
		//ZeroMemory((char*)&buffer, 4);
		int kplen = this->kpLength;
		/*for (int j = 0; j < kplen; j++) {
			ZeroMemory(buf, 25000);
			ZeroMemory((char*)&buffer, 4);

			for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv failed");
			kp.pt.x = buffer;
			ZeroMemory((char*)&buffer,4);

			for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv2 failed");
			kp.pt.y = buffer;
			ZeroMemory((char*)&buffer, 4);

			for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv3 failed");
			kp.size = buffer;

			for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv3 failed");
			kp.angle = buffer;
			ZeroMemory((char*)&buffer, 4);

			for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv3 failed");
			kp.class_id = buffer;
			ZeroMemory((char*)&buffer, 4);

			for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv3 failed");
			kp.octave = buffer;
			ZeroMemory((char*)&buffer, 4);


			for (int i = 0; i < 4; i += bytes)
			if ((bytes = recv(clientSocket, (char*)&buffer+i, 4-i, 0)) == -1) cout << ("recv3 failed");
			kp.response = buffer;
			ZeroMemory((char*)&buffer, 4);

			veckp.push_back(kp);
		}*/
		//cout <<"KeyPoints: "<< veckp[0].pt.x << endl;

		ZeroMemory(buf, sizeof(buf));
		//for (int i = 0; i < len; i += bytes)
		if ((bytes = recv(clientSocket,buf, sizeof(buf), 0)) == -1) cout << ("recv3 failed");
		KeyPoint *p;

		for (p = (KeyPoint*)&buf[0]; p <= (KeyPoint*)&buf[bytes-1]; p++) {
			veckp.push_back(*p);
			//*p++;
		}
		p=NULL;

		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}

		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}
		if (port == 53000) {
			this->m_kpC = veckp;
			imgK = this->imgCenter.clone();
			drawKeypoints(imgK, this->m_kpC, imgK);
			cv::imshow("Frame with Center-Keypoints", imgK);
			if (waitKey(1000 / 20) >= 0) 
			{
				break;
			}
		}
		else if (port == 59000) {
			this->m_kpL = veckp;
			//mu2.lock();
			//drawKeypoints(this->imgLeft, this->m_kpL, this->imgLeft);
			//mu2.unlock();
			//cv::imshow("Frame with LeftKeypoints", this->imgLeft);
			//if (waitKey(1000 / 20) >= 0) {
			//	break;
			//}
		}
		/*Mat dsc;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		detector->compute(this->imgCenter, veckp, dsc);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/ 1000000.0 << "[Sekunden]" << std::endl;*/


	}
	closesocket(clientSocket);

	WSACleanup();
}

void Stitching::getFrameTCP(int port,int imgNumber, String windowname) {
	int height = 480;
	int width = 640;
	int bytes = 0;
	SOCKET clientSocket = createSocket(port);
	KeyPoint kp;
	//int buffsize = 848 * 480 * 3;
	//setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	char buf[921600];
	//char buf[848*480*3];

	while (true)
	{
		Mat img = Mat::zeros(height, width, CV_8UC3);
		int imgSize;
		imgSize = img.total()*img.elemSize();
		ZeroMemory(buf, imgSize);
		//Wait for client to send data
		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(clientSocket, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		int ptr = 0;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
				ptr = ptr + 3;
				//cout << buf[ptr + 0] << endl << buf[ptr + 1] << endl << buf[ptr + 2] << endl;

			}
		}
		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}
		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}

		//Mat frame(640, 480, CV_8UC3);
		//frame= imdecode(buffer, IMREAD_COLOR);
		//send(clientSocket, response, sizeof(response) , 0);
		//img =imdecode(buffer, IMREAD_COLOR);
	//drawKeypoints(img, veckp, img);
		if (port == 54000) {
				this->imgCenter = img.clone();
			//	namedWindow(windowname, WINDOW_FREERATIO);
			//	imshow(windowname, this->imgCenter);

			//if (waitKey(1000 / 10) >= 0) {
			//	break;
			//}
		}
		else if (port == 58000) {
				mu2.lock();
				this->imgLeft = img.clone();
				mu2.unlock();
				//namedWindow(windowname, WINDOW_FREERATIO);
				//imshow(windowname, this->imgLeft);
	
			//if (waitKey(1000 / 10) >= 0) {
			//	break;
			//}
		}

	}
	closesocket(clientSocket);

	WSACleanup();
}

Mat Stitching::getAlpha(Mat dst1, Mat dst2) {
	typedef float  Float32;
	typedef long float  Float64;
	Mat alpha = Mat::zeros(Size(dst1.size()), dst1.type());


	for (int r = 0; r < dst1.rows; r++) {
		for (int c = 0; c < dst2.cols; c++) {
			if (r < dst1.rows&&c < dst1.cols)
			{
				alpha.at<float>(r, c) = Float64(dst1.at<float>(r, c) / (dst1.at<float>(r, c) + dst2.at<float>(r, c)) + 0.00000000001);

			}
			else {
				alpha.at<float>(r, c) = 0;
			}
			//cout << "dst2\t" << dst2.at<float>(r, c) << endl;
			//cout << "alpha\t" << alpha.at<float>(r, c) << endl;

		}
	}
	return alpha;
}

Mat Stitching::getBeta(Mat dst1, Mat dst2) {
	typedef float  Float32;
	typedef long float  Float64;
	Mat beta = Mat::zeros(Size(dst1.size()), dst1.type());
	//cout << "dst1\t" << beta.at<float>(dst1.rows-1, dst1.cols-1) << endl;

	for (int r = 0; r < dst1.rows; r++) {
		for (int c = 0; c < dst1.cols; c++) {
			beta.at<float>(dst1.rows - 1 - r, dst1.cols - 1 - c) = Float64(dst1.at<float>(r, c) / (dst1.at<float>(r, c) + dst2.at<float>(r, c) + 0.0000000000000001));

			//cout << "dst1\t" << beta.at<float>(dst1.rows-r-1, dst1.cols-c-1) << endl;
			//cout << "dst2\t" << dst2.at<float>(r, c) << endl;
			//cout << "alpha\t" << alpha.at<float>(r, c) << endl;

		}
	}
	return beta;
}

void Stitching::takeDFT(Mat &source, Mat &destination)
{
	Mat originalComplex[2] = { source, Mat::zeros(source.size(),CV_32F) };
	Mat dftReady;
	merge(originalComplex, 2, dftReady);
	Mat dftOfOriginal;
	dft(dftReady, dftOfOriginal, DFT_COMPLEX_OUTPUT);
	destination = dftOfOriginal;

}

void Stitching::showDFT(Mat &source)
{
	Mat splitArray[2] = { Mat::zeros(source.size(),CV_32F),Mat::zeros(source.size(),CV_32F) };
	split(source, splitArray);
	Mat dftMagnitude;
	magnitude(splitArray[0], splitArray[1], dftMagnitude);
	dftMagnitude += Scalar::all(1);
	log(dftMagnitude, dftMagnitude);
	normalize(dftMagnitude, dftMagnitude, 0, 1, NORM_MINMAX);

}

Mat Stitching::makeDFT(Mat I) {

	cvtColor(I, I, COLOR_BGR2GRAY);
	Mat padded;                            //expand input image to optimal size
	int m = getOptimalDFTSize(I.rows);
	int n = getOptimalDFTSize(I.cols); // on the border add zero values
	copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));
	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);         // Add to the expanded another plane with zeros
	dft(complexI, complexI);            // this way the result may fit in the source matrix
	// compute the magnitude and switch to logarithmic scale
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
	Mat magI = planes[0];
	magI += Scalar::all(1);                    // switch to logarithmic scale
	log(magI, magI);
	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));
	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;
	Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right
	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);
	normalize(magI, magI, 0, 1, NORM_MINMAX); // Transform the matrix with float values into a
											// viewable image form (float between values 0 and 1).
	//imshow("Input Image", I);    // Show the result
	//imshow("spectrum magnitude", magI);
	return magI;
}
void Stitching::recenterDFT(Mat &source) {
	int centerX = source.cols / 2;
	int centerY = source.rows / 2;

	Mat q1(source, Rect(0, 0, centerX, centerY));
	Mat q2(source, Rect(centerX, 0, centerX, centerY));
	Mat q3(source, Rect(0, centerY, centerX, centerY));
	Mat q4(source, Rect(centerX, centerY, centerX, centerY));

	Mat swapMap;
	q1.copyTo(swapMap);
	q4.copyTo(q1);
	swapMap.copyTo(q4);

	q2.copyTo(swapMap);
	q3.copyTo(q2);
	swapMap.copyTo(q3);
}

void Stitching::inverseDFT(Mat &source, Mat &destination) {
	Mat inverse;
	dft(source, inverse, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
	destination = inverse;
}

Rect Stitching::findbiggestContour(Mat img) {
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	cvtColor(img, img, COLOR_BGR2GRAY);
	threshold(img, img, 0, 255, THRESH_BINARY);
	findContours(img, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_NONE);
	int largest_area = 0;
	int largest_contour_index = 0;
	Rect rct;
	for (int i = 0; i < contours.size(); i++)
	{
		//  Find the area of contour
		double a = contourArea(contours[i], false);
		if (a > largest_area) {
			largest_area = a;
			cout << i << " area  " << a << endl;
			// Store the index of largest contour
			largest_contour_index = i;
			// Find the bounding rectangle for biggest contour
			rct = boundingRect(contours[largest_contour_index]);
		}
	}
	return rct;
}

Mat Stitching::rotate(Mat src, double angle)   //rotate function returning mat object with parametres imagefile and angle    
{
	Mat dst;      //Mat object for output image file
	Point2f pt(src.cols / 2., src.rows / 2.);          //point from where to rotate    
	Mat r = getRotationMatrix2D(pt, angle, 1.0);      //Mat object for storing after rotation
	warpAffine(src, dst, r, Size(src.cols, src.rows));  ///applie an affine transforation to image.
	return dst;         //returning Mat object for output image file
}

void Stitching::findMatches() {
	this->imgLeft = imgLeft;
	this->imgRight = imgRight;
	this->imgCenter = imgCenter;
	this->imgBottom1 = imgBottom1;
	Mat grayLimage, grayRimage, grayCenter, grayBottom1, grayBottom2;
	vector <Mat> imgSum;
	Size size(imgLeft.cols / 1.5, imgLeft.rows / 1.5);
	flip(imgBottom1, imgBottom1, -1);
	resize(imgLeft, imgLeft, size);
	resize(imgRight, imgRight, size);
	resize(imgCenter, imgCenter, size);
	resize(imgBottom1, imgBottom1, size);
	cvtColor(imgLeft, grayLimage, COLOR_RGB2GRAY);
	cvtColor(imgRight, grayRimage, COLOR_RGB2GRAY);
	cvtColor(imgCenter, grayCenter, COLOR_RGB2GRAY);
	cvtColor(imgBottom1, grayBottom1, COLOR_RGB2GRAY);
	vector <Mat> images;
	images.push_back(imgLeft);
	images.push_back(imgRight);
	images.push_back(imgCenter);
	images.push_back(imgBottom1);
	vector <KeyPoint> kpL, kpR, kpC, kpB1;
	Mat result;
	Mat descriptorL, descriptorR, descriptorC, descriptorB1;
	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	f2d->detect(imgCenter, kpC);
	f2d->detect(imgBottom1, kpB1);
	f2d->detect(imgLeft, kpL);
	f2d->detect(imgRight, kpR);
	f2d->compute(imgLeft, kpL, descriptorL);
	f2d->compute(imgRight, kpR, descriptorR);
	f2d->compute(imgCenter, kpC, descriptorC);
	f2d->compute(imgBottom1, kpR, descriptorB1);


	BFMatcher matcher;
	BFMatcher matcher1(NORM_L2, true);
	vector< DMatch > matches1, matches2;
	matcher.match(descriptorL, descriptorR, matches1);
	matcher.match(descriptorC, descriptorB1, matches2);

	Mat matKp, matKp1;
	drawKeypoints(imgLeft, kpL, matKp);
	drawKeypoints(imgRight, kpR, matKp);
	drawKeypoints(imgCenter, kpC, matKp1);
	drawKeypoints(imgBottom1, kpC, matKp1);
	Mat matMatches1, matMatches2;
	drawMatches(imgLeft, kpL, imgRight, kpR, matches1, matMatches1);
	drawMatches(imgCenter, kpC, imgBottom1, kpB1, matches2, matMatches2);

	this->m_kpL = m_kpL;
	this->m_kpR = m_kpR;
	this->m_kpC = m_kpC;
	this->m_kpB1 = m_kpB1;

	Mat siftdescriptorL, siftdescriptorR, siftdescriptorC, siftdescriptorB1;
	Ptr<SIFT> sift = SIFT::create();
	sift->detectAndCompute(imgLeft, noArray(), m_kpL, siftdescriptorL);
	sift->detectAndCompute(imgRight, noArray(), m_kpR, siftdescriptorR);
	sift->detectAndCompute(imgCenter, noArray(), m_kpC, siftdescriptorC);
	sift->detectAndCompute(imgBottom1, noArray(), m_kpB1, siftdescriptorB1);

	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> matcher11;
	matcher11 = BFMatcher::create(NORM_L2, false);
	vector<vector<DMatch>> matchesknn, matchesknn2;
	matcher11->knnMatch(siftdescriptorL, siftdescriptorR, matchesknn, 2);
	matcher11->knnMatch(siftdescriptorC, siftdescriptorB1, matchesknn2, 2);
	this->mt1 = mt1;
	this->mt2 = mt2;
	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < 0.75 * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}
	for (int i = 0; i < matchesknn2.size(); i++) {
		if (matchesknn2[i][0].distance < 0.75 * matchesknn2[i][1].distance) {
			mt2.push_back(matchesknn2[i][0]);
		}
	}
	//cout << matches1[1].size();
	Mat outputMatches, outputMatches1;
	drawMatches(imgLeft, m_kpL, imgRight, m_kpR, mt1, outputMatches);
	drawMatches(imgCenter, m_kpC, imgBottom1, m_kpB1, mt2, outputMatches1);

	imshow("matches", outputMatches);
	imshow("matchesBottomTop", outputMatches1);

	waitKey();
}


void Stitching::makePanorama(int i = 0) {
	this->imgLeft = imgLeft;
	this->imgRight = imgRight;
	this->m_kpC = m_kpC;
	this->m_kpB1 = m_kpB1;
	vector<Point2f> obj1, obj2;
	vector<Point2f> scene1, scene2;
	this->m_kpL = m_kpL;
	this->m_kpR = m_kpR;
	this->m_kpC = m_kpC;
	this->m_kpB1 = m_kpB1;
	this->mt1 = mt1;
	this->mt2 = mt2;

	for (int i = 0; i < mt1.size(); i++) {
		obj1.push_back(m_kpL[mt1[i].queryIdx].pt);
		scene1.push_back(m_kpR[mt1[i].trainIdx].pt);

	}

	for (int i = 0; i < mt2.size(); i++) {
		obj2.push_back(m_kpC[mt2[i].queryIdx].pt);
		scene2.push_back(m_kpB1[mt2[i].trainIdx].pt);

	}

	Mat HomoMatrix = findHomography(scene1, obj1, RANSAC);
	Mat HomoMatrix2 = findHomography(scene2, obj2, RANSAC);

	Mat result1, result2;
	warpPerspective(imgRight, result1, HomoMatrix, Size(imgLeft.cols + imgRight.cols, imgRight.rows), INTER_CUBIC);
	warpPerspective(imgBottom1, result2, HomoMatrix2, Size(imgCenter.cols + imgCenter.cols, imgCenter.rows), INTER_CUBIC);

	Mat panorama, panorama1;
	panorama = result1.clone();
	panorama1 = result2.clone();

	Mat roi(panorama, Rect(0, 0, imgLeft.cols, imgLeft.rows));
	Mat roi1(panorama1, Rect(0, 0, imgCenter.cols, imgCenter.rows));

	imgLeft.copyTo(roi);
	imgCenter.copyTo(roi1);
	if (i == 0) {
		imshow("panorama", panorama);
		imshow("panorama1", panorama1);

		waitKey();
	}
}

Mat Stitching::stitching(Mat img1, Mat img2) {
	Stitcher::Mode mode = Stitcher::PANORAMA;
	Mat pano;
	vector<Mat> imgs;
	imgs.push_back(img1);
	imgs.push_back(img2);
	//imgs.push_back(img3);
	//imgs.push_back(img4);
	Ptr<Stitcher> stitcher = Stitcher::create(mode);
	Stitcher::Status status = stitcher->stitch(imgs, pano);
	if (status != 0) {
		cout << "Stitching nicht erfolgreich wegen Error-Code" << int(status) << endl;
	}
	//stitcher->estimateTransform(imgs);
	//stitcher->composePanorama(pano);
	//imshow("panoramaStitching", pano);
	//waitKey();
	return pano;

}
vector <KeyPoint> Stitching::getCurrentKeypoints(Mat img) {
	vector <KeyPoint> kp;

	if (norm(img, this->imgLeft, NORM_L1) == false) {
		kp = this->m_kpL;

	}
	else if (norm(img, this->imgRight, NORM_L1) == false) {
		kp = this->m_kpR;
	}
	else if (norm(img, this->imgCenter, NORM_L1) == false) {
		kp = this->m_kpC;
	}
	else if (norm(img, this->imgBottom1, NORM_L1) == false) {
		kp = this->m_kpB1;
	}
	else if (norm(img, this->imgBottom2, NORM_L1) == false) {
		kp = this->m_kpB2;
	}
	else {
		kp = this->m_kptemp;
	}
	return kp;
}

void Stitching::setKeypoints(Mat img, vector <KeyPoint> kp) {

	if (norm(img, this->imgLeft, NORM_L1) == false) {
		this->m_kpL = kp;

	}
	else if (norm(img, this->imgRight, NORM_L1) == false) {
		this->m_kpR = kp;
	}
	else if (norm(img, this->imgCenter, NORM_L1) == false) {

		this->m_kpC = kp;
	}
	else if (norm(img, this->imgBottom1, NORM_L1) == false) {
		this->m_kpB1 = kp;
	}
	else if (norm(img, this->imgBottom2, NORM_L1) == false) {
		this->m_kpB2 = kp;
	}
	else
	{
		this->m_kptemp = kp;
	}

}

Mat Stitching::makeNormalize(Mat img) {

	Mat grayImg, bin, dst;
	if (img.type() != 0) {
		cvtColor(img, grayImg, COLOR_BGR2GRAY);
	}
	else {
		grayImg = img;
	}

	//removeBlackPoints(grayImg);

	cuda::GpuMat grayImgGPU, binGPU, dstGPU;
	grayImgGPU.upload(grayImg);
	cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);
	//threshold(grayImg, bin, 0, 255, THRESH_BINARY);
	binGPU.download(bin);
	//imshow("bin", bin);
	//Mat kernel = Mat::ones(Size(55, 55), dst.type());
	//dilate(bin, bin, kernel, Point(-1, -1), 2);

	distanceTransform(bin, dst, DIST_L2, 3.0);
	double min, max;
	cv::minMaxLoc(bin, &min, &max);
	//imshow("dst", dst);
	//waitKey(0);
	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));

	//erode(dst, dst, kernel, Point(-1, -1), 2);
	dstGPU.upload(dst);
	/*double alpha = 0.;
	cuda::normalize(dstGPU, dstGPU, alpha, (1.-alpha), NORM_MINMAX,-1);*/


	dstGPU.download(dst);


	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));
	//dilate(dst, dst, kernel,Point(-1,-1));

	return dst;
}

vector<DMatch> Stitching::getSiftmatches(Mat &img1, Mat &img2, float a)
{

	vector <KeyPoint> kp1, kp2;
	Mat siftdescriptor1, siftdescriptor2;
	Ptr<SIFT> sift = SIFT::create();

	sift->detectAndCompute(img1, noArray(), kp1, siftdescriptor1);
	sift->detectAndCompute(img2, noArray(), kp2, siftdescriptor2);
	//cout << "reale Datei: " << kp1[0].pt.x << endl;
	//cout <<"KP: "<< (char*)kp1.data() << endl;
	//char *buf = (char*)kp1.data();
	//KeyPoint *p = (KeyPoint*)buf;
	//cout << "BUF: " << p->pt.x << endl;
	//vector<KeyPoint> kpoints;

	//for (int i = 0; i < kp1.size(); i++) {
	//	kpoints.push_back(*p++);
	//}
	//cout <<"ENCODEE"<< kpoints[35].pt.x << endl;
	//cout << "reale Datei: " << kp1[35].pt.x << endl;

	//cout << "KPsize: " << kp1.max_size() << endl;
	//KeyPoint *buffer;
	//char *c = (char*)kp1.data();
	//buffer = (KeyPoint*)c;

	//KeyPoint *p = buffer;
	//cout << "P: " << (p+1)->pt.x << endl;

	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> matcher11;
	matcher11 = BFMatcher::create(NORM_L2, false);
	vector<vector<DMatch>> matchesknn, matchesknn2;
	matcher11->knnMatch(siftdescriptor1, siftdescriptor2, matchesknn, 2);

	vector<DMatch> mt1;

	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}


	setKeypoints(img1, kp1);
	setKeypoints(img2, kp2);
	return mt1;

}

void Stitching::showatches(Mat img1, Mat img2, vector <KeyPoint> kp1, vector <KeyPoint> kp2, vector<DMatch> matches)
{
	Mat outputMatches;
	drawMatches(img1, kp1, img2, kp2, matches, outputMatches);
	resize(outputMatches, outputMatches, Size(1024, 800));
	imshow("Matches", outputMatches);
	waitKey();
}

Mat Stitching::getHomography(Mat img1, Mat img2, vector <DMatch> matches)
{
	vector<KeyPoint> kpObj, kpScn;
	kpObj = getCurrentKeypoints(img1);
	kpScn = getCurrentKeypoints(img2);
	vector<Point2f> obj, scene;
	for (int i = 0; i < matches.size(); i++) {
		if ((matches[i].queryIdx < kpObj.size()) && (matches[i].trainIdx < kpScn.size())) {
			obj.push_back(kpObj[matches[i].queryIdx].pt);
			scene.push_back(kpScn[matches[i].trainIdx].pt);
		}

	}
	Mat H = findHomography(scene, obj, RANSAC);
	return H;
}


void Stitching::CudaVideoStream(String path, String windowname, double fps, cuda::GpuMat &frameGPU, int a) {
	VideoCapture cap(path);
	//cap.set(CAP_PROP_POS_FRAMES,10);
	Mat frame;
	while (cap.isOpened()) {
		mu.lock();
		if (a == 1) {
			//cap.read(frame);
			cap.grab();
			cap.retrieve(frame);
			resize(frame, frame, Size(848, 480));
			flip(frame, frame, -1);
			frameGPU.upload(frame);
			mu.unlock();
		}
		else {
			cap.read(frame);
			resize(frame, frame, Size(848, 480));
			frameGPU.upload(frame);
			mu.unlock();
		}
		namedWindow(windowname, WINDOW_OPENGL);

		imshow(windowname, frameGPU);

		if (waitKey(1000 / fps) >= 0) {
			break;
		}
	}
}

void Stitching::CudaVideoStreamAll(String path1, String path2, String path3, String path4, String path5) {
	VideoCapture cap1(path1);
	VideoCapture cap2(path2);
	VideoCapture cap3(path3);
	VideoCapture cap4(path4);
	VideoCapture cap5(path5);

	Mat frame1, frame2, frame3, frame4, frame5;
	while (true) {
		mu.lock();
		cap1.read(frame1);
		cap2.read(frame2);
		cap3.read(frame3);
		cap4.read(frame4);
		cap5.read(frame5);

		Size sz = Size(640, 480);
		Size sz2 = sz / 4;

		//resize(frame1, frame1, sz);
		//resize(frame2, frame2, sz);
		//resize(frame3, frame3, sz);
		//resize(frame4, frame4, sz);
		//resize(frame5, frame5, sz);



		Mat centerTemplate = Mat::zeros(Size(frame1.size() * 2), frame1.type());
		Mat rightTemplate = centerTemplate.clone();
		Mat leftTemplate = centerTemplate.clone();
		Mat bottom1Template = centerTemplate.clone();
		Mat bottom2Template = centerTemplate.clone();
		Mat roi(centerTemplate, Rect(frame1.cols / 2, frame1.rows / 2, frame1.cols, frame1.rows));
		Mat roi1(rightTemplate, Rect(frame1.cols / 2, frame1.rows / 2, frame1.cols, frame1.rows));
		Mat roi2(leftTemplate, Rect(frame1.cols / 2, frame1.rows / 2, frame1.cols, frame1.rows));
		Mat roi3(bottom1Template, Rect(frame1.cols / 2, frame1.rows / 2, frame1.cols, frame1.rows));
		Mat roi4(bottom2Template, Rect(frame1.cols / 2, frame1.rows / 2, frame1.cols, frame1.rows));
		frame4.copyTo(roi);
		frame2.copyTo(roi2);
		frame1.copyTo(roi1);
		frame3.copyTo(roi3);
		frame5.copyTo(roi4);

		this->imgCenter = centerTemplate;
		this->imgRight = rightTemplate;
		this->imgLeft = leftTemplate;
		this->imgBottom1 = bottom1Template;
		this->imgBottom2 = bottom2Template;
		frame1 = rightTemplate;
		frame2 = leftTemplate;
		frame3 = bottom1Template;
		frame4 = centerTemplate;
		frame5 = bottom2Template;

		imgRightGPU.upload(frame1);
		imgLeftGPU.upload(frame2);
		imgBottom1GPU.upload(frame3);
		imgBottom2GPU.upload(frame5);
		imgCenterGPU.upload(frame4);
		mu.unlock();

		this_thread::sleep_for(0.01s);

		//namedWindow("frame1", WINDOW_OPENGL);
		//namedWindow("frame2", WINDOW_OPENGL);
		//namedWindow("frame3", WINDOW_OPENGL);
		//namedWindow("frame4", WINDOW_OPENGL);
		//imshow("frame1", imgRightGPU);


		//imshow("frame2", imgLeftGPU);
		//imshow("frame3", imgBottom1GPU);
		//imshow("frame4", imgCenterGPU);
		//if (waitKey(1000 / 10) >= 0) {
		//	break;
		//}

	}
}
void Stitching::CudaVideoStream2(String path, String windowname, double fps, cuda::GpuMat &frameGPU, int a) {
	VideoCapture cap(path);
	//cap.set(CAP_PROP_POS_FRAMES,10);
	Mat frame;
	while (cap.isOpened()) {
		mu2.lock();
		if (a == 1) {
			cap.read(frame);
			resize(frame, frame, Size(848, 480));
			flip(frame, frame, -1);
			frameGPU.upload(frame);
			mu2.unlock();
		}
		else {
			cap.read(frame);
			resize(frame, frame, Size(848, 480));
			frameGPU.upload(frame);
			mu2.unlock();
		}
		namedWindow(windowname, WINDOW_OPENGL);

		imshow(windowname, frameGPU);

		if (waitKey(1000 / fps) >= 0) {
			break;
		}
	}
}


void Stitching::CudaVideoStream3(String path, String windowname, double fps, cuda::GpuMat &frameGPU, int a) {
	VideoCapture cap(path);
	//cap.set(CAP_PROP_POS_FRAMES,10);
	Mat frame;
	while (cap.isOpened()) {
		mu3.lock();
		if (a == 1) {
			cap.read(frame);
			resize(frame, frame, Size(848, 480));
			flip(frame, frame, -1);
			frameGPU.upload(frame);
			mu3.unlock();
		}
		else {
			cap.read(frame);
			resize(frame, frame, Size(848, 480));
			frameGPU.upload(frame);
			mu3.unlock();
		}
		namedWindow(windowname, WINDOW_OPENGL);

		imshow(windowname, frameGPU);

		if (waitKey(1000 / fps) >= 0) {
			break;
		}
	}
}

void Stitching::CudaVideoStream4(String path, String windowname, double fps, cuda::GpuMat &frameGPU, int a) {
	VideoCapture cap(path);
	//cap.set(CAP_PROP_POS_FRAMES,10);
	Mat frame;
	while (cap.isOpened()) {
		mu4.lock();
		if (a == 1) {
			cap.read(frame);
			resize(frame, frame, Size(848, 480));
			flip(frame, frame, -1);
			frameGPU.upload(frame);
			mu4.unlock();
		}
		else {
			cap.read(frame);
			resize(frame, frame, Size(848, 480));
			frameGPU.upload(frame);
			mu4.unlock();
		}
		namedWindow(windowname, WINDOW_OPENGL);

		imshow(windowname, frameGPU);

		if (waitKey(1000 / fps) >= 0) {
			break;
		}
	}
}

void Stitching::VideoStream(String path, String windowname, double fps, Mat &frame) {
	VideoCapture cap(path);

	while (cap.isOpened()) {
		mu.lock();
		cap.read(frame);
		resize(frame, frame, Size(848, 480));
		mu.unlock();
		namedWindow(windowname, WINDOW_FREERATIO);

		imshow(windowname, frame);

		if (waitKey(1000 / fps) >= 0) {
			break;
		}
	}
}


Mat Stitching::getPerspectiveMat(Mat img1, Mat img2, vector <DMatch> matches, Mat H)
{
	m_kpL = this->m_kpL;
	m_kpR = this->m_kpR;
	vector<Point2f> obj, scene;
	Point2f ptobj[44];
	Point2f scobj[44];

	for (int i = 0; i < matches.size(); i++) {
		obj.push_back(m_kpL[matches[i].queryIdx].pt);
		scene.push_back(m_kpR[matches[i].trainIdx].pt);
		ptobj[i] = obj[i];
		scobj[i] = scene[i];

	}
	vector<Point2f> pts;
	Size k = img1.size();
	pts.push_back(Point2f(0, 0));
	pts.push_back(Point2f(0, k.height - 1));
	pts.push_back(Point2f(k.width - 1, k.height - 1));
	pts.push_back(Point2f(k.width, 0));

	vector<Point2f> pts1;
	perspectiveTransform(pts, pts1, H);
	Mat s;
	vector<Point> ptp;
	for (int i = 0; i < pts1.size(); i++) {
		ptp.push_back(pts1[i]);
	}
	polylines(img1, ptp, true, Scalar(255, 255, 255));
	Mat P = getPerspectiveTransform(ptobj, scobj);
	return P;
}

Mat Stitching::makePanorama2(Mat &img1, Mat &img2, Mat HomoMatrix)
{
	for (;;) {
		Mat result;
		resize(img1, img1, Size(848, 480));
		resize(img2, img2, Size(848, 480));
		while (true) {
			std::this_thread::sleep_for(4s);

			Mat zer = Mat::zeros(Size(img1.cols + img2.cols, img2.rows + img1.rows), img1.type());
			Mat roiarea(zer, Rect(0, 0, img1.cols, img1.rows));
			Mat roiarea2(zer, Rect(0, 0, img1.cols + img2.cols, img1.rows));

			warpPerspective(img2, result, HomoMatrix, Size(img1.cols + img2.cols, img1.rows + img2.rows), INTER_CUBIC);

			if (norm(img2, this->imgLeft, NORM_L1) == false) {
				this->warpLeft = result;

			}
			else if (norm(img2, this->imgRight, NORM_L1) == false) {
				this->warpRight = result;
				cout << "Geschafft" << endl;
			}
			if (norm(img2, this->imgBottom1, NORM_L1) == false) {
				this->warpDown = result;
			}
			if (norm(img2, this->imgBottom2, NORM_L1) == false) {
				this->warpDown2 = result;
			}
			namedWindow("gewarptes Bild", WINDOW_FREERATIO);
			imshow("gewarptes Bild", result);

			if (waitKey(1000 / 20) >= 0) {
				break;
			}
		}
		Mat panorama;
		panorama = result.clone();
		Mat roi(panorama, Rect(0, 0, imgLeft.cols, imgLeft.rows));
		//panorama.copyTo(roiarea2);
		img1.copyTo(roi);
		imshow("panorama", panorama);
		waitKey();
		return result;
	}

}

void Stitching::warpImage(cuda::GpuMat &img1GPU, cuda::GpuMat &img2GPU, Mat h, String WindowName) {
	this_thread::sleep_for(2s);
	for (;;) {
		Mat result;
		Mat img1, img2;
		cuda::GpuMat resultGPU;

		while (true) {
			mu.lock();
			img1GPU.download(img1);
			img2GPU.download(img2);

			warpPerspective(img2, result, h, Size(img1.cols, img1.rows), INTER_CUBIC);

			resultGPU.upload(result);
			mu.unlock();

			if (norm(h, this->hL, NORM_L1) == false) {
				this->warpLeftGPU = resultGPU;

			}
			else if (norm(h, this->hR, NORM_L1) == false) {
				this->warpRightGPU = resultGPU;
			}
			else if (norm(h, this->hB1, NORM_L1) == false) {
				this->warpDownGPU = resultGPU;

			}
			else if (norm(h, this->hB2, NORM_L1) == false) {
				this->warpDown2GPU = resultGPU;
			}
			//namedWindow(WindowName, WINDOW_OPENGL);
			//imshow(WindowName , resultGPU);

			//if (waitKey(1000 / 20) >= 0) {
			//	break;
			//}
		}

	}
}
vector<DMatch> Stitching::getOrbmatches(Mat &img1, Mat &img2, double a)
{
	vector <KeyPoint> kp1, kp2;
	Mat grayimg1, grayimg2;
	cvtColor(img1, grayimg1, COLOR_BGR2GRAY);
	cvtColor(img2, grayimg2, COLOR_BGR2GRAY);

	Mat orbdescriptor1, orbdescriptor2;
	//Ptr<ORB> orb = ORB::create();
	Ptr<FeatureDetector> detector = ORB::create();
	Ptr<DescriptorExtractor> descriptor = ORB::create();
	//orb->detectAndCompute(grayimg1, noArray(), kp1, orbdescriptor1);
	//orb->detectAndCompute(grayimg2, noArray(), kp2, orbdescriptor2);
	detector->detect(grayimg1, kp1);
	detector->detect(grayimg2, kp2);

	descriptor->compute(grayimg1, kp1, orbdescriptor1);
	descriptor->compute(grayimg2, kp2, orbdescriptor2);


	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> bfmatcher;
	bfmatcher = BFMatcher::create(NORM_HAMMING, true);
	vector<DMatch> matches;
	bfmatcher->match(orbdescriptor1, orbdescriptor2, matches);

	this->m_kpL = kp1;
	this->m_kpR = kp2;
	setKeypoints(img1, kp1);
	setKeypoints(img2, kp2);
	return matches;
}

void Stitching::CudaStitch(cuda::GpuMat &img1, cuda::GpuMat &img2, cuda::GpuMat &img3, cuda::GpuMat &img4) {
	for (;;) {
		this_thread::sleep_for(5s);

		while (true) {
			//mu.lock();
			Mat img2CPU;
			img2.download(img2CPU);
			Mat centerTemplate = Mat::zeros(Size(img2CPU.size()), img2CPU.type());
			cuda::GpuMat centerTemplateGPU;
			centerTemplateGPU.upload(centerTemplate);
			cuda::GpuMat roiCenter(centerTemplateGPU, Rect(0, 0, img1.cols, img1.rows));
			img1.copyTo(roiCenter);
			Rect cropLast(imgCenterGPU.cols / 5, imgCenterGPU.rows / 1.4, imgCenterGPU.cols / 1.5, imgCenterGPU.rows - imgCenterGPU.rows / 1.35);

			centerTemplateGPU.download(centerTemplate);

			//for (int r = 0; r < centerTemplate.rows; r++) {
			//	for (int c = 0; c < centerTemplate.cols; c++) {
			//		if (cropLast.contains(Point(c, r)) == true) {
			//			centerTemplate.at<Vec3b>(r, c) = Vec3b(0, 0, 0);
			//		}
			//	}
			//}

			Mat dstcenter = makeNormalize(centerTemplate);
			Mat dstright = makeNormalize(img2CPU);

			Mat alpha1 = getAlpha(dstcenter, dstright);
			Mat firstpano = Mat::zeros(Size(centerTemplate.size()), centerTemplate.type());

			//Erstes Alpha Blending (Mittleres und linkes Bild)
			for (int r = 0; r < dstcenter.rows; r++) {
				for (int c = 0; c < dstcenter.cols; c++) {

					//firstpano.at<uchar>(r, c) = alpha1.at<float>(r, c) * (int)upgray.at<uchar>(r, c) + (1. - alpha1.at<float>(r, c)) * (int)lowgray.at<uchar>(r, c);
					firstpano.at<Vec3b>(r, c)[0] = alpha1.at<float>(r, c) * centerTemplate.at<Vec3b>(r, c)[0] + (1. - alpha1.at<float>(r, c)) * img2CPU.at<Vec3b>(r, c)[0];
					firstpano.at<Vec3b>(r, c)[1] = alpha1.at<float>(r, c) * centerTemplate.at<Vec3b>(r, c)[1] + (1. - alpha1.at<float>(r, c)) * img2CPU.at<Vec3b>(r, c)[1];
					firstpano.at<Vec3b>(r, c)[2] = alpha1.at<float>(r, c) * centerTemplate.at<Vec3b>(r, c)[2] + (1. - alpha1.at<float>(r, c)) * img2CPU.at<Vec3b>(r, c)[2];

				}
			}
			centerTemplateGPU.upload(centerTemplate);
			//cuda::GpuMat firstpanoGPU;
			//firstpanoGPU.upload(firstpano);
			Mat img3CPU;
			img3.download(img3CPU);
			Mat dstleft = makeNormalize(img3CPU);
			Mat fpano = makeNormalize(firstpano);
			Mat alpha2 = getAlpha(fpano, dstleft);
			Mat secondpano = Mat::zeros(Size(centerTemplate.size()), centerTemplate.type());
			for (int r = 0; r < dstright.rows; r++) {
				for (int c = 0; c < dstright.cols; c++) {

					//secondpano.at<uchar>(r, c) = alpha2.at<float>(r, c) * (int)firstpano.at<uchar>(r, c) + (1. - alpha2.at<float>(r, c)) * (int)graywarpRight.at<uchar>(r, c);
					secondpano.at<Vec3b>(r, c)[0] = alpha2.at<float>(r, c) * firstpano.at<Vec3b>(r, c)[0] + (1. - alpha2.at<float>(r, c)) * img3CPU.at<Vec3b>(r, c)[0];
					secondpano.at<Vec3b>(r, c)[1] = alpha2.at<float>(r, c) * firstpano.at<Vec3b>(r, c)[1] + (1. - alpha2.at<float>(r, c)) * img3CPU.at<Vec3b>(r, c)[1];
					secondpano.at<Vec3b>(r, c)[2] = alpha2.at<float>(r, c) * firstpano.at<Vec3b>(r, c)[2] + (1. - alpha2.at<float>(r, c)) * img3CPU.at<Vec3b>(r, c)[2];

				}
			}
			cuda::GpuMat secondpanoGPU;
			secondpanoGPU.upload(secondpano);
			Mat img4CPU;
			img3.download(img4CPU);
			Mat dstdown = makeNormalize(img4CPU);
			normalize(dstdown, dstdown, 0., 1., NORM_MINMAX);
			Mat fpano2 = makeNormalize(secondpano);
			Mat alpha3 = getAlpha(fpano2, dstdown);
			Mat thirdpano = Mat::zeros(Size(centerTemplate.size()), centerTemplate.type());
			for (int r = 0; r < dstright.rows; r++) {
				for (int c = 0; c < dstright.cols; c++) {

					//thirdpano.at<uchar>(r, c) = alpha3.at<float>(r, c) * (int)secondpano.at<uchar>(r, c) + (1. - alpha3.at<float>(r, c)) * (int)graywarpDown.at<uchar>(r, c);
					thirdpano.at<Vec3b>(r, c)[0] = alpha3.at<float>(r, c) * secondpano.at<Vec3b>(r, c)[0] + (1. - alpha3.at<float>(r, c)) * img4CPU.at<Vec3b>(r, c)[0];
					thirdpano.at<Vec3b>(r, c)[1] = alpha3.at<float>(r, c) * secondpano.at<Vec3b>(r, c)[1] + (1. - alpha3.at<float>(r, c)) * img4CPU.at<Vec3b>(r, c)[1];
					thirdpano.at<Vec3b>(r, c)[2] = alpha3.at<float>(r, c) * secondpano.at<Vec3b>(r, c)[2] + (1. - alpha3.at<float>(r, c)) * img4CPU.at<Vec3b>(r, c)[2];

				}
			}
			cuda::GpuMat thirdpanoGPU;
			thirdpanoGPU.upload(thirdpano);
			//mu.unlock();

			namedWindow("gestitched", WINDOW_OPENGL);
			imshow("gestitched", thirdpanoGPU);
			if (waitKey(1000 / 20) >= 0) {
				break;
			}
		}
	}
}

vector<DMatch> Stitching::getSiftmatchesFlann(Mat img1, Mat img2, float ratio_thresh)
{
	vector <KeyPoint> kp1, kp2;
	Mat siftdescriptor1, siftdescriptor2;
	Ptr<SIFT> sift = SIFT::create();
	sift->detectAndCompute(img1, noArray(), kp1, siftdescriptor1);
	sift->detectAndCompute(img2, noArray(), kp2, siftdescriptor2);
	int FLANN_INDEX_KDTREE = 0;

	Ptr<FlannBasedMatcher> flann = FlannBasedMatcher::create();

	//Ptr<cv::DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
	vector<vector<DMatch>> matchesknn;
	flann->knnMatch(siftdescriptor1, siftdescriptor2, matchesknn, 2);


	std::vector<DMatch> good_matches;
	for (size_t i = 0; i < matchesknn.size(); i++)
	{
		if (matchesknn[i][0].distance < ratio_thresh * matchesknn[i][1].distance)
		{
			good_matches.push_back(matchesknn[i][0]);
		}
	}

	this->m_kpL = kp1;
	this->m_kpR = kp2;
	setKeypoints(img1, kp1);
	setKeypoints(img2, kp2);
	return good_matches;

}
Mat Stitching::AlphaBlending(Mat img, Mat addImg) {
	Mat dst1 = makeNormalize(img);

	Mat dst2 = makeNormalize(addImg);
	Mat alpha = getAlpha(dst1, dst2);
	//Mat beta = getBeta(dst1, dst2);
	Mat Pano = Mat::zeros(Size(img.size()), img.type());

	//Erstes Alpha Blending (Mittleres und linkes Bild)
	for (int r = 0; r < dst1.rows; r++)
	{
		for (int c = 0; c < dst1.cols; c++)
		{

			//firstpano.at<uchar>(r, c) = alpha1.at<float>(r, c) * (int)upgray.at<uchar>(r, c) + (1. - alpha1.at<float>(r, c)) * (int)lowgray.at<uchar>(r, c);
			Pano.at<Vec3b>(r, c)[0] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[0] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[0];
			Pano.at<Vec3b>(r, c)[1] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[1] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[1];
			Pano.at<Vec3b>(r, c)[2] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[2] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[2];

		}
	}
	return Pano;
}
void Stitching::StitchAllImgs(cuda::GpuMat &warpBottom1GPU, cuda::GpuMat &warpBottom2GPU, cuda::GpuMat &warpRightGPU, cuda::GpuMat &warpLeftGPU)
{
	this_thread::sleep_for(3s);
	while (true)
	{
		/*	this->imgCenterGPU.download(this->imgCenter);
			Rect cropLast(this->imgCenter.cols / 5, this->imgCenter.rows / 1.4, this->imgCenter.cols / 1.5, this->imgCenter.rows - this->imgCenter.rows / 1.4);
			Mat centerTemplate = Mat::zeros(Size(this->imgCenter.size()*2), this->imgCenter.type());
			Mat roi(centerTemplate, Rect(0, 0, this->imgCenter.cols, this->imgCenter.rows));
			this->imgCenter.copyTo(roi);
	*/
	//for (int r = 0; r < centerTemplate.rows; r++) {
	//	for (int c = 0; c < centerTemplate.cols; c++) {
	//		if (cropLast.contains(Point(c, r)) == true) {
	//			centerTemplate.at<Vec3b>(r, c) = Vec3b(0, 0, 0);
	//		}
	//	}
	//}

	//#########################################################STITCHING###########################################################//

		Mat wBottom1, wBottom2, wRight, wLeft;
		warpBottom1GPU.download(wBottom1);
		warpBottom2GPU.download(wBottom2);
		warpRightGPU.download(wRight);
		warpLeftGPU.download(wLeft);
		Mat centerTemplate = this->imgCenter;
		Mat fstPano = AlphaBlending(centerTemplate, wBottom1);
		Mat scnPano = AlphaBlending(fstPano, wBottom2);
		Mat thdPano = AlphaBlending(scnPano, wRight);
		lastPano = AlphaBlending(thdPano, wLeft);
		cuda::GpuMat lastPanoGPU;
		lastPanoGPU.upload(lastPano);

		namedWindow("gestitched", WINDOW_OPENGL);
		imshow("gestitched", lastPanoGPU);
		if (waitKey(1000 / 60) >= 0) {
			break;
		}
		wBottom1.release();
		wBottom2.release();

		wRight.release();
		wLeft.release();

	}
}

void Stitching::streamRealSense()
{
	rs2::context ctx;        // Create librealsense context for managing devices

	std::map<std::string, rs2::colorizer> colorizers; // Declare map from device serial number to colorizer (utility class to convert depth data RGB colorspace)

	std::vector<rs2::pipeline>  pipelines;

	// Capture serial numbers before opening streaming
	std::vector<std::string>              serials;
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}


	rs2::pipeline pipe(ctx), pipe2(ctx), pipe3(ctx), pipe4(ctx), pipe5(ctx);
	rs2::config cfg, cfg2, cfg3, cfg4, cfg5;
	cfg.enable_device(serials[0]);
	cfg2.enable_device(serials[1]);
	cfg3.enable_device(serials[2]);
	cfg4.enable_device(serials[3]);
	cfg5.enable_device(serials[4]);

	cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg2.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg3.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg4.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg5.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);

	pipe.start(cfg);
	pipe2.start(cfg2);
	pipe3.start(cfg3);
	pipe4.start(cfg4);
	pipe5.start(cfg5);

	pipelines.emplace_back(pipe);
	pipelines.emplace_back(pipe2);
	pipelines.emplace_back(pipe3);
	pipelines.emplace_back(pipe4);
	pipelines.emplace_back(pipe5);

	colorizers[serials[0]] = rs2::colorizer();
	colorizers[serials[1]] = rs2::colorizer();
	colorizers[serials[2]] = rs2::colorizer();
	colorizers[serials[3]] = rs2::colorizer();
	colorizers[serials[4]] = rs2::colorizer();

	std::map<int, rs2::frame> render_frames;
	rs2::frameset framesOCV;
	rs2::frame color_frame0;
	rs2::frame color_frame1;
	rs2::frame color_frame2;
	rs2::frame color_frame3;
	rs2::frame color_frame4;
	VideoWriter video("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam0.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video1("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam1.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video2("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam2.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video3("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam3.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video4("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam4.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));

	while (true) {
		color_frame0 = pipelines[0].wait_for_frames();
		color_frame1 = pipelines[1].wait_for_frames();
		color_frame2 = pipelines[2].wait_for_frames();
		color_frame3 = pipelines[3].wait_for_frames();
		color_frame4 = pipelines[4].wait_for_frames();

		Mat color0(Size(640, 480), CV_8UC3, (void*)color_frame0.get_data(), Mat::AUTO_STEP);
		Mat color1(Size(640, 480), CV_8UC3, (void*)color_frame1.get_data(), Mat::AUTO_STEP);
		Mat color2(Size(640, 480), CV_8UC3, (void*)color_frame2.get_data(), Mat::AUTO_STEP);
		Mat color3(Size(640, 480), CV_8UC3, (void*)color_frame3.get_data(), Mat::AUTO_STEP);
		Mat color4(Size(640, 480), CV_8UC3, (void*)color_frame4.get_data(), Mat::AUTO_STEP);
		video.write(color0);
		video1.write(color1);
		video2.write(color2);
		video3.write(color3);
		video4.write(color4);

		//this->imgLeftGPU.upload(color0);
		//this->imgRightGPU.upload(color1);
		//this->imgRightGPU.upload(color2);
		//this->imgRightGPU.upload(color3);
		//this->imgRightGPU.upload(color4);

		namedWindow("cam0", WINDOW_FREERATIO);
		namedWindow("cam1", WINDOW_FREERATIO);
		namedWindow("cam2", WINDOW_FREERATIO);
		namedWindow("cam3", WINDOW_FREERATIO);
		namedWindow("cam4", WINDOW_FREERATIO);

		imshow("cam0", color0);
		imshow("cam1", color1);
		imshow("cam2", color2);
		imshow("cam3", color3);
		imshow("cam4", color4);

		if (waitKey(1000 / 20) >= 0) {
			break;
		}
	}
	//VideoCapture cap(serials[0]);
	//Mat frame;
	//while (cap.isOpened()) {
	//	mu.lock();
	//	cap.read(frame);
	//	resize(frame, frame, Size(848, 480));
	//	mu.unlock();
	//	namedWindow("Stream", WINDOW_FREERATIO);

	//	imshow("Stream", frame);

	//	if (waitKey(1000 / 20) >= 0) {
	//		break;
	//	}
	//}
}


vector<Mat> Stitching::takeCapture()
{
	Size sz1 = Size(848, 480);
	Size sz2 = Size(640, 480);
	rs2::context ctx;        // Create librealsense context for managing devices
	std::vector<std::string> serials;
	rs2::rates_printer printer;
	rs2::colorizer color_map;

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}
	vector<rs2::config> cfg;
	for (int i = 0; i < serials.size(); i++) {
		rs2::config cfgtemp;
		cfgtemp.enable_device(serials[i]);
		cfgtemp.enable_stream(RS2_STREAM_COLOR, sz2.width, sz2.height, RS2_FORMAT_BGR8, 30);
		cfg.push_back(cfgtemp);
	}

	vector<Mat> imgs;
	rs2::pipeline pipe1, pipe2;
	std::vector<rs2::pipeline>  pipelines;
	for (int i = 0; i < cfg.size(); i++) {
		rs2::pipeline pipe;
		pipelines.push_back(pipe);
	}

	for (int i = 0; i < pipelines.size(); i++) {


		pipelines[i].start(cfg[i]);
	}
	////Instruct pipeline to start streaming with the requested configuration
	//pipe1.start(cfg1);
	//pipe2.start(cfg2);
	// Camera warmup - dropping several first frames to let auto-exposure stabilize
	rs2::frameset frames1, frames2, frames3, frames4, frames5;
	vector<rs2::frameset> frame;
	for (int i = 0; i < 30; i++)
	{
		frames1 = pipelines.at(0).wait_for_frames();
		frames2 = pipelines.at(1).wait_for_frames();
		frames3 = pipelines.at(2).wait_for_frames();
		frames4 = pipelines.at(3).wait_for_frames();
		frames5 = pipelines.at(4).wait_for_frames();

	}

	//Get each frame
	rs2::frame color_frame1 = frames1.get_color_frame();
	rs2::frame color_frame2 = frames2.get_color_frame();
	rs2::frame color_frame3 = frames3.get_color_frame();
	rs2::frame color_frame4 = frames4.get_color_frame();
	rs2::frame color_frame5 = frames5.get_color_frame();

	// Creating OpenCV Matrix from a color image
	Mat color1(sz2, CV_8UC3, (void*)color_frame1.get_data(), Mat::AUTO_STEP);
	Mat color2(sz2, CV_8UC3, (void*)color_frame2.get_data(), Mat::AUTO_STEP);
	Mat color3(sz2, CV_8UC3, (void*)color_frame3.get_data(), Mat::AUTO_STEP);
	Mat color4(sz2, CV_8UC3, (void*)color_frame4.get_data(), Mat::AUTO_STEP);
	Mat color5(sz2, CV_8UC3, (void*)color_frame5.get_data(), Mat::AUTO_STEP);

	// Display in a GUI
	//namedWindow("Display Image1", WINDOW_AUTOSIZE);
	//imshow("Display Image1", color1);
	//waitKey(0);
	//imshow("Display Image2", color2);
	//imshow("Display Image3", color3);
	//imshow("Display Image4", color4);
	//imshow("Display Image5", color5);

	//waitKey(0);
	imgs.push_back(color1); //Left
	imgs.push_back(color2); //Right
	imgs.push_back(color3);	//BottomLeft
	imgs.push_back(color4);	//Center
	imgs.push_back(color5);	//BottomRight
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\LeftUp4.png", color1);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\RightUp4.png", color2);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\centerUp4.png", color3);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\BottomLeftUp4.png", color4);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\BottomRightUp4.png", color5);
	cout << "FERTIG" << endl;
	return imgs;
}


vector<DMatch> Stitching::getSureMatches(Mat imgCPU1, Mat imgCPU2, double a)
{
	Mat imgGray1, imgGray2;

	cvtColor(imgCPU1, imgGray1, COLOR_BGR2GRAY);
	cvtColor(imgCPU2, imgGray2, COLOR_BGR2GRAY);
	//Ptr<cuda::SURF_CUDA> surf;
	cuda::SURF_CUDA surf;
	// detecting keypoints & computing descriptors
	cuda::GpuMat img1, img2, keypoints1GPU, keypoints2GPU;
	cuda::GpuMat descriptors1GPU, descriptors2GPU;
	img1.upload(imgGray1);
	img2.upload(imgGray2);
	surf(img1, cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
	surf(img2, cuda::GpuMat(), keypoints2GPU, descriptors2GPU);
	vector<float> dsc;
	surf.downloadDescriptors(descriptors1GPU, dsc);

//	float *p = dsc.data();
//	for (int i = 0; i < dsc.size(); i++) {
//	cout << *p++ << endl;
//	if (i == 20) {
//		break;
//
//	}
//}
//	cout << "Richtiger Vektor" << endl;
//	for (int i = 0; i < dsc.size(); i++) {
//		cout << dsc[i] << endl;
//		if (i == 20) {
//			break;
//		}
//	}

	//surf->detectWithDescriptors(img1, cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
	//surf->detectWithDescriptors(img2, cuda::GpuMat(), keypoints2GPU, descriptors2GPU);
/*
	Ptr<cv::cuda::DescriptorMatcher> matcher = cuda::DescriptorMatcher::createBFMatcher(surf.defaultNorm());
	vector<DMatch> matches;
	matcher->match(descriptors1GPU, descriptors2GPU, matches);

	vector<KeyPoint> keypoints1, keypoints2;
	vector<float> descriptors1, descriptors2;
	surf.downloadKeypoints(keypoints1GPU, keypoints1);
	surf.downloadKeypoints(keypoints2GPU, keypoints2);
	surf.downloadDescriptors(descriptors1GPU, descriptors1);
	surf.downloadDescriptors(descriptors2GPU, descriptors2);
	Mat img_matches;
	vector<DMatch> mt1;
	vector<vector<DMatch>> matchesknn;
	matcher->knnMatch(descriptors1GPU, descriptors2GPU, matchesknn, 2);
	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}*/
	//drawMatches(Mat(img1), keypoints1, Mat(img2), keypoints2, mt1, img_matches);

	//imwrite("seaman_result.jpg", img_matches);

	//namedWindow("matches", 0);
	//imshow("matches", img_matches);
	//waitKey(0);

	//cuda::resetDevice();
	/*vector <KeyPoint> kp1, kp2;
	Mat suredescriptor1, suredescriptor2;
	Ptr<cuda::SURF_CUDA> surf = cuda::SURF_CUDA::create(200);

	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> matcher11;
	matcher11 = BFMatcher::create(NORM_L2, false);
	vector<vector<DMatch>> matchesknn, matchesknn2;
	matcher11->knnMatch(siftdescriptor1, siftdescriptor2, matchesknn, 2);

	vector<DMatch> mt1;

	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}*/


	//setKeypoints(imgCPU1, keypoints1);
	//setKeypoints(imgCPU2, keypoints2);
	return mt1;
}

vector<DMatch> Stitching::getSurfMatches(Mat img1, Mat img2, double a, double b, int minHessian)
{
	Mat imgGray1, imgGray2;
	cvtColor(img1, imgGray1, COLOR_BGR2GRAY);
	cvtColor(img2, imgGray2, COLOR_BGR2GRAY);
	Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<KeyPoint> kp1, kp2;
	Mat surfdescriptor1, surfdescriptor2;

	detector->detectAndCompute(imgGray1, noArray(), kp1, surfdescriptor1);
	detector->detectAndCompute(imgGray2, noArray(), kp2, surfdescriptor2);

	


	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> matcher11;
	matcher11 = BFMatcher::create(NORM_L2, false);
	vector<vector<DMatch>> matchesknn, matchesknn2;
	matcher11->knnMatch(surfdescriptor1, surfdescriptor2, matchesknn, 2);
	vector<DMatch> mt1;

	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}
	setKeypoints(img1, kp1);
	setKeypoints(img2, kp2);
	if (b != 0)
	{

		Mat matchesimg1, matachesimg2;
		drawMatches(img1, kp1, img2, kp2, mt1, matchesimg1);
		namedWindow("matches", WINDOW_FREERATIO);
		imshow("matches", matchesimg1);
		waitKey(0);
	}
	return mt1;

}



Rect Stitching::lastRct(Mat img, int a) {
	Mat blurCenter, centerGray, centerthresh, image;
	medianBlur(img, blurCenter, 185);
	cvtColor(blurCenter, centerGray, COLOR_BGR2GRAY);
	threshold(centerGray, centerthresh, 164, 255, THRESH_BINARY);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(centerthresh, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	int largest_area = 0;
	int largest_contour_index = 0;
	for (int i = 0; i < contours.size(); i++) {
		if (hierarchy[i][3] == -1) {
			//drawContours(centerTemplate, contours, i, (255, 0, 0), 10);
		}
		double af = contourArea(contours[i], false);
		if (af > largest_area) {
			largest_area = af;
			cout << i << " area  " << af << endl;
			// Store the index of largest contour
			largest_contour_index = i;
		}
	}
	Rect rct12 = boundingRect(contours[largest_contour_index]);
	rectangle(img, rct12, Scalar(255, 255, 255), 10);
	if (a == 1) {
		imshow("Rechteck um Last", img);
		waitKey(0);
	}
	return rct12;
}

void Stitching::cropImg(Mat &img, Rect rct, int d)
{

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			int dX = c - rct.x;
			int dY = r - rct.y;
			int dX2 = rct.width - c;
			int dY2 = rct.height - r;

			if (((dX > d&&dY > d) && (dX2 < d&&dY2 < d)) && rct.contains(Point(c, r))) {
				img.at<Vec3b>(r, c) = Vec3b(0, 0, 0);
			}
		}
	}
}

void Stitching::removeBlackPoints(Mat &img)
{
	vector<Point> nonBlackList2;
	nonBlackList2.reserve((int)img.rows + (int)img.cols);

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if ((int)img.at<uchar>(r, c) != 0) {

				nonBlackList2.push_back(Point(c, r));
			}
		}
	}
	Rect nonBlacklistRect = boundingRect(nonBlackList2);
	img = img(nonBlacklistRect);

}