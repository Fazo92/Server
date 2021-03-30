#pragma once
#include <iostream>
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include <librealsense2/rs.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/dnn.hpp>
#include <iomanip>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/h/rs_option.h>
#include <librealsense2/h/rs_frame.h>
#include <stack>
#include <cassert>
#include "server.h"
#include <opencv2/core/cuda.hpp>


#define STREAM          RS2_STREAM_DEPTH  // rs2_stream is a types of data provided by RealSense device           //
#define FORMAT          RS2_FORMAT_Z16    // rs2_format identifies how binary data is encoded within a frame      //
#define WIDTH           640               // Defines the number of columns for each frame or zero for auto resolve//
#define HEIGHT          0                 // Defines the number of lines for each frame or zero for auto resolve  //
#define FPS             30                // Defines the rate of frames per second                                //
#define STREAM_INDEX    0                 // Defines the stream index, used for multiple streams of the same type //

using namespace std;
using namespace cv;
using namespace rs2;
using namespace cv::dnn;

class rsFeature {
	public:
	Mat getcolorFrameRS(rs2::pipeline pipe);
    rs2::pipeline setInfraredRS (int width, int height);
    Mat getInfraredRS(rs2::pipeline pipe);
    rs2::pipeline getColorFrameRSpipeline();
    void getDistance();
    void check_error(rs2_error* e)
    {
        if (e)
        {
            printf("rs_error was raised when calling %s(%s):\n", rs2_get_failed_function(e), rs2_get_failed_args(e));
            printf("    %s\n", rs2_get_error_message(e));
            exit(EXIT_FAILURE);
        }
    }
    void detectObject(Mat& img, Mat depthImage);
    Mat detecedObject;
    Rect rct;
    Mat getDepthFrame(rs2::pipeline pipe, rs2::colorizer color_map);
    void RegionGrowth(Mat& src);
    void sendCamFrame(int port);
    void getWarpedImg(int port);
    void getdepth_and_colorFrame(bool isRunning, Mat& img1, Mat& depth_img);

    private:
        int width, height;
        void grow(cv::Mat& src, cv::Mat& dest, cv::Mat& mask, cv::Point seed, int threshold);





        // parameters
        const int threshold = 200;
        const uchar max_region_num = 100;
        const double min_region_area_factor = 0.01;
        const cv::Point PointShift2D[8] =
        {
            cv::Point(1, 0),
            cv::Point(1, -1),
            cv::Point(0, -1),
            cv::Point(-1, -1),
            cv::Point(-1, 0),
            cv::Point(-1, 1),
            cv::Point(0, 1),
            cv::Point(1, 1)
        };
};