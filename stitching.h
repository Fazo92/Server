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

public:
	//Initialisieren der Funktionen. In der .cpp-Datei werden die Funktionen implementiert
	static void makeNormalizethread(cuda::GpuMat img, cuda::GpuMat& dst_gpu);
	cuda::GpuMat getAlphaGPU(cuda::GpuMat dst1, cuda::GpuMat dst2);
	static void AlphaBlendingGPU(Mat img, Mat addImg, Mat& pano);

};