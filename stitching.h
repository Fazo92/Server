#pragma once
#include <iostream>
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/core.hpp"
#include "opencv2/stitching.hpp"
#include <opencv2/cudaarithm.hpp>
#include <chrono>
#include <mutex>
#include <map> 
#include <opencv2/xfeatures2d/cuda.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include "opencv2/opencv_modules.hpp"
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <chrono>
#include <opencv2/videoio.hpp>
#include <librealsense2/rs.hpp>
#include <opencv2/cudawarping.hpp>
//#include <opencv2/flann.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudafeatures2d.hpp>

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/cudafeatures2d.hpp"
#include "opencv2/xfeatures2d/cuda.hpp"
#include "rsFeature.h"
#pragma comment (lib,"ws2_32.lib")



using namespace std;
using namespace cv;
using namespace xfeatures2d;

class Stitching {
	Mat img, dsctmp;
	vector<KeyPoint> kptmp, kptmpright,kptmpleft,kptmpcenter,kptmpbottom;
	Mat frametmpright,frametmpleft,frametmpcenter,frametmpbottom;
	bool features = false;
	bool rs = false;
public:
	//server serv;
	cuda::GpuMat dscGPUnew;
	Mat dscnew1;
	Mat dscRight, dscCenter, dscLeft,dscBottom;
	Mat camFrame;

	int dscSize = 1000;
	Mat imgLeft, imgRight, imgCenter, imgBottom1, imgBottom2, hR, hL, hB1, hB2, warpLeft, warpRight, warpDown, warpDown2;
	cuda::GpuMat imgLeftGPU, imgRightGPU, imgCenterGPU, imgBottom1GPU, imgBottom2GPU, warpLeftGPU, warpRightGPU, warpDownGPU, warpDown2GPU;


	vector <KeyPoint> m_kpL, m_kpR, m_kpC, m_kpB1, m_kpB2, m_kptemp;
	vector <DMatch> mt1, mt2;
	//Stitching(String path1, String path2, String path3, String path4);
	//Stitching();


	Rect findbiggestContour(Mat img);

	
	Mat lastPano;
	void streamRealSense();

	Mat makeNormalize(Mat img);
	static void makeNormalizethread(cuda::GpuMat img, cuda::GpuMat& dst_gpu);

	cuda::GpuMat getAlphaGPU(cuda::GpuMat dst1, cuda::GpuMat dst2);
	static void AlphaBlendingGPU(Mat img, Mat addImg,Mat &pano);
private:
	cuda::GpuMat m_alpha1, m_beta1, m_alpha2, m_beta2, m_alpha3, m_beta3;
	int counter = 0;
	static cv::Mat frame_to_mat(const rs2::frame& f)
	{
		using namespace cv;
		using namespace rs2;

		auto vf = f.as<video_frame>();
		const int w = vf.get_width();
		const int h = vf.get_height();

		if (f.get_profile().format() == RS2_FORMAT_BGR8)
		{
			return Mat(Size(w, h), CV_8UC3, (void*)f.get_data(), Mat::AUTO_STEP);
		}
		else if (f.get_profile().format() == RS2_FORMAT_RGB8)
		{
			auto r_rgb = Mat(Size(w, h), CV_8UC3, (void*)f.get_data(), Mat::AUTO_STEP);
			Mat r_bgr;
			cv::cvtColor(r_rgb, r_bgr, COLOR_RGB2BGR);
			return r_bgr;
		}
		else if (f.get_profile().format() == RS2_FORMAT_Z16)
		{
			return Mat(Size(w, h), CV_16UC1, (void*)f.get_data(), Mat::AUTO_STEP);
		}
		else if (f.get_profile().format() == RS2_FORMAT_Y8)
		{
			return Mat(Size(w, h), CV_8UC1, (void*)f.get_data(), Mat::AUTO_STEP);
		}
		else if (f.get_profile().format() == RS2_FORMAT_DISPARITY32)
		{
			return Mat(Size(w, h), CV_32FC1, (void*)f.get_data(), Mat::AUTO_STEP);
		}

		throw std::runtime_error("Frame format is not supported yet!");
	}

	static cv::Mat depth_frame_to_meters(const rs2::depth_frame& f1)
	{

		cv::Mat dm = frame_to_mat(f1);
		//dm.convertTo(dm, CV_64F);
		//dm = dm * f1.get_units();
		//dm = dm / 1000.;
		return dm;
	}
};