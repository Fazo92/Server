#include "Stitching.h"
//#include <chrono>

typedef float  Float32;
typedef long float  Float64;

void main() {


	//cuda::setDevice(0);
	Stitching st;

	////
	thread t1(&Stitching::getFrameTCP, &st,54000,0, "Bild 1");
	//////thread t11(&Stitching::getFrameTCP, &st, 58000, 0, "Bild 2");

	thread t2(&Stitching::getKeyPointsTCP, &st,53000);
	////thread t22(&Stitching::getKeyPointsTCP, &st, 59000);

	//thread t3(&Stitching::getCudaDescriptorsTCP, &st, 52000);
	thread t3(&Stitching::getDescriptorTCP, &st, 52000);
	//thread t33(&Stitching::getDescriptorTCP, &st,60000);

	//thread t4(&Stitching::getCUDADimensions, &st, 51000);
	//thread t4(&Stitching::getDimensions, &st, 51000);
	//thread t44(&Stitching::getDimensions, &st,61000);

	//thread t5(&Stitching::realTimeStitching, &st);

	t1.join();
	//t11.join();
	t2.join();
	//t22.join();
	t3.join();
	////t44.join();
	//t4.join();
	//t5.join();

	//st.imgRight = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\RightUp.png", IMREAD_COLOR);
	//st.imgCenter = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\centerUp.png", IMREAD_COLOR);
	//st.imgBottom2 = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\BottomLeftUp.png", IMREAD_COLOR);
	//st.imgBottom1 = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\BottomRightUp.png", IMREAD_COLOR);
	//st.imgLeft = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\Aufnahmen_11012021\\fork_Left1_Color.png", IMREAD_COLOR);
	//st.imgRight = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\Aufnahmen_11012021\\fork_Right1_Color.png", IMREAD_COLOR);
	//st.imgCenter = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\Aufnahmen_11012021\\fork_Center1_Color.png", IMREAD_COLOR);
	//st.imgBottom2 = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\Aufnahmen_11012021\\fork_BottomRight1_alternative_Color.png", IMREAD_COLOR);
	//st.imgBottom1 = imread("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\Aufnahmen_11012021\\fork_BottomLeft1_alternative_Color.png", IMREAD_COLOR);
	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	////////////SURF####################################
	double ratio = 0.6;

	//vector < DMatch > matchesCL = st.getSurfMatches(st.imgCenter, st.imgLeft, ratio);
	//vector < DMatch > matchesCR = st.getSurfMatches(st.imgCenter, st.imgRight, ratio);
	////vector < DMatch > matchesCB1 = st.getSurfMatches(st.imgCenter, st.imgBottom1, ratio);
	////vector < DMatch > matchesCB2 = st.getSurfMatches(st.imgCenter, st.imgBottom2, ratio);


	//////////SIFT####################################
		//vector < DMatch > matchesCL = st.getSiftmatches(st.imgCenter, st.imgLeft, ratio);
	//vector < DMatch > matchesCR = st.getSiftmatches(st.imgCenter, st.imgRight, ratio);
	//vector < DMatch > matchesCB1 = st.getSiftmatches(st.imgCenter, st.imgBottom1, ratio);
	//vector < DMatch > matchesCB2 = st.getSiftmatches(st.imgCenter, st.imgBottom2, ratio);

		//vector < DMatch > matchesCR = st.getSureMatches(st.imgCenter, st.imgRight, ratio);


	////vector < DMatch > matchesCL = st.getSiftmatchesFlann(st.imgCenter, st.imgLeft, ratio);
	////vector < DMatch > matchesCR = st.getSiftmatchesFlann(st.imgCenter, st.imgRight, ratio);
	////vector < DMatch > matchesCB1 = st.getSiftmatchesFlann(st.imgCenter, st.imgBottom1, ratio);
	////vector < DMatch > matchesCB2 = st.getSiftmatchesFlann(st.imgCenter, st.imgBottom2, ratio);
	//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/ 1000000.0 << "[Sekunden]" << std::endl;


	//Mat hCL = st.getHomography(st.imgCenter, st.imgLeft, matchesCL);
	//Mat hCR = st.getHomography(st.imgCenter, st.imgRight, matchesCR);
	//Mat hCB1 = st.getHomography(st.imgCenter, st.imgBottom1, matchesCB1);
	//Mat hCB2 = st.getHomography(st.imgCenter, st.imgBottom2, matchesCB2);

	//Mat centerTemplate = Mat::zeros(Size(st.imgCenter.size() * 2), st.imgCenter.type());
	//Mat rightTemplate = centerTemplate.clone();
	//Mat leftTemplate = centerTemplate.clone();
	//Mat bottom1Template = centerTemplate.clone();
	//Mat bottom2Template = centerTemplate.clone();
	//Mat roi(centerTemplate, Rect(st.imgCenter.cols / 2, st.imgCenter.rows / 2, st.imgCenter.cols, st.imgCenter.rows));
	//Mat roi1(rightTemplate, Rect(st.imgCenter.cols / 2, st.imgCenter.rows / 2, st.imgCenter.cols, st.imgCenter.rows));
	//Mat roi2(leftTemplate, Rect(st.imgCenter.cols / 2, st.imgCenter.rows / 2, st.imgCenter.cols, st.imgCenter.rows));
	//Mat roi3(bottom1Template, Rect(st.imgCenter.cols / 2, st.imgCenter.rows / 2, st.imgCenter.cols, st.imgCenter.rows));
	//Mat roi4(bottom2Template, Rect(st.imgCenter.cols / 2, st.imgCenter.rows / 2, st.imgCenter.cols, st.imgCenter.rows));
	//st.imgCenter.copyTo(roi);
	//st.imgRight.copyTo(roi1);
	//st.imgLeft.copyTo(roi2);
	//st.imgBottom1.copyTo(roi3);
	//st.imgBottom2.copyTo(roi4);

	//st.imgCenter = centerTemplate;
	//st.imgRight = rightTemplate;
	//st.imgLeft = leftTemplate;
	//st.imgBottom1 = bottom1Template;
	//st.imgBottom2 = bottom2Template;

	//
	//st.hR = hCR;
	//st.hL = hCL;
	//st.hB1 = hCB1;
	//st.hB2 = hCB2;

	//Mat wLeft, wRight, wBottom1, wBottom2;
	//warpPerspective(leftTemplate, wLeft, hCL, Size(st.imgLeft.size()), INTER_CUBIC);

	//warpPerspective(rightTemplate, wRight, hCR, Size(st.imgLeft.size()), INTER_CUBIC);

	//warpPerspective(bottom1Template, wBottom1, hCB1, Size(st.imgLeft.size()), INTER_CUBIC);

	//warpPerspective(bottom2Template, wBottom2, hCB2, Size(st.imgLeft.size()), INTER_CUBIC);
	//

	//Mat fstPano = st.AlphaBlending(centerTemplate, wBottom1);
	//Mat scnPano = st.AlphaBlending(fstPano, wBottom2);
	//Mat thdPano = st.AlphaBlending(scnPano, wRight);
	//Mat lPano = st.AlphaBlending(thdPano, wLeft);
	//Rect rctPano = st.findbiggestContour(lPano);
	//namedWindow("lPano", WINDOW_FREERATIO);
	//imshow("lPano", lPano(rctPano));

	//waitKey(0);


	//String path1 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\LinkeKamera\\left4_Color.png";
	//String path2 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\RechteKamera\\right4_Color.png";
	//String path3 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\MittlereKamera\\center4_Color.png";
	//String path4 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\KameraUnten\\down4_Color.png";
	//st.imgLeft = imread(path1,IMREAD_COLOR);
	//st.imgRight = imread(path2, IMREAD_COLOR);
	//st.imgCenter = imread(path3, IMREAD_COLOR);
	//st.imgBottom1 = imread(path4, IMREAD_COLOR);
	//st.imgBottom2 = imread(path4, IMREAD_COLOR);

//	vector < DMatch > matchesCR = st.getSurfMatches(st.imgCenter, st.imgRight, 0.75);
//	vector < DMatch > matchesCL = st.getSurfMatches(st.imgCenter, st.imgLeft, 0.75);
//	vector < DMatch > matchesCB1 = st.getSurfMatches(st.imgCenter, st.imgBottom1, 0.75);
//	vector < DMatch > matchesCB2 = st.getSurfMatches(st.imgCenter, st.imgBottom2, 0.75);
//
//	Mat hCR = st.getHomography(st.imgCenter, st.imgRight, matchesCR);
//	Mat hCL = st.getHomography(st.imgCenter, st.imgLeft, matchesCL);
//	Mat hCB1 = st.getHomography(st.imgCenter, st.imgBottom1, matchesCB1);
//	Mat hCB2 = st.getHomography(st.imgCenter, st.imgBottom2, matchesCB2);
//
//	st.hR = hCR;
//st.hL = hCL;
//st.hB1 = hCB1;
//st.hB2 = hCB2;
////	//Mat hCB2 = st.getHomography(centerTemplate, bottom2Template, matchesCB2);
//	///////////---------Rechtes Bild
//	String adress1 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam1.avi";
//	///////////---------linkes Bild
//	String adress2 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam0.avi";
//	///////////--------- unteres rechtes Bild
//	String adress3 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam3.avi";
//
//	String adress5 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam2.avi";
//
//	///////////---------mittleres Bild
//	String adress4 = "C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam4.avi";
////
////
//		thread t1(&Stitching::CudaVideoStreamAll, &st, adress1, adress2, adress3, adress4, adress5);
//		thread t5(&Stitching::warpImage, &st, ref(st.imgCenterGPU), ref(st.imgRightGPU), hCR,"warpRight");
//		thread t6(&Stitching::warpImage, &st, ref(st.imgCenterGPU), ref(st.imgLeftGPU), hCL,"warpLeft");
//		thread t7(&Stitching::warpImage, &st, ref(st.imgCenterGPU), ref(st.imgBottom1GPU), hCB1, "warpbottom");
//		thread t8(&Stitching::warpImage, &st, ref(st.imgCenterGPU), ref(st.imgBottom2GPU), hCB2, "warpbottom2");
////
//		thread t9(&Stitching::StitchAllImgs, &st, ref(st.warpDownGPU), ref(st.warpDownGPU), ref(st.warpRightGPU), ref(st.warpLeftGPU));
////		////thread t8(&Stitching::warpImage, &st, ref(st.imgCenterGPU), ref(st.imgBottom2GPU), hCL, "warpLeft");
////
//		t1.join();
//		t5.join();
//		t6.join();
//		t7.join();
//		t8.join();
//		t9.join();
}