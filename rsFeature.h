#pragma once
#include <iostream>
//#include "opencv2/calib3d.hpp"
//#include "opencv2/highgui.hpp"
//#include "opencv2/core.hpp"
#include <librealsense2/rs.hpp>
#include <librealsense2/rs.h>

//#include <librealsense2/rs.hpp>
//#include <opencv2/video/tracking.hpp>
//#include <opencv2/dnn.hpp>
#include <iomanip>
//#include <librealsense2/h/rs_pipeline.h>
//#include <librealsense2/h/rs_option.h>
//#include <librealsense2/h/rs_frame.h>
#include <stack>
#include <cassert>
#include "stitching.h"
#include "sock.h"
#include <fstream>

//#include <opencv2/core/cuda.hpp>


#define STREAM          RS2_STREAM_DEPTH  // rs2_stream is a types of data provided by RealSense device           //
#define FORMAT          RS2_FORMAT_Z16    // rs2_format identifies how binary data is encoded within a frame      //
#define WIDTH           640               // Defines the number of columns for each frame or zero for auto resolve//
#define HEIGHT          0                 // Defines the number of lines for each frame or zero for auto resolve  //
#define FPS             30                // Defines the rate of frames per second                                //
#define STREAM_INDEX    0                 // Defines the stream index, used for multiple streams of the same type //

using namespace std;
using namespace cv;
using namespace rs2;

class rsFeature {
public:
    Mat getcolorFrameRS(rs2::pipeline pipe);
    rs2::pipeline setInfraredRS(int width, int height);
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
    vector<Rect> rct_vec;
    Mat getDepthFrame(rs2::pipeline pipe, rs2::colorizer color_map);
    void RegionGrowth(Mat& src);
    void sendCamFrame(int port);
    void getWarpedImg(int port);
    void getdepth_and_colorFrame(bool &isRunning, Mat& img1, Mat& depth_img, String deviceNumber);
    Mat m_img,m_d_img, m_d_imgUp,pano_depth,pano_img,m_imgR, m_imgL, m_imgBL, m_imgBR, m_DimgR, m_DimgL, m_DimgBL, m_DimgBR;
    Mat m_imgB, m_dimgB,d_img_Range;
    Mutex mu_img_send, mu_imgR_send, mu_imgB_send,mu_d_img_Range;
    Mutex mu_dimg, mu_dimgR, mu_dimgL, mu_dimgBR, mu_dimgBL;
    void sichteinschränkungsdetektion(sock socket);
    void reference_cam(String deviceNumber);
    void holo_cam(String deviceNumber);
    float hindernisVal=0.;
    bool hindernis=false;
    void recv_img(int port, string IP);
    void recv_depth_img(int port, string IP);
    Mat getMatinRange(Mat depth_img, double minRange = 0.3, double maxRange = 1.5);
    Mutex mu_wR, mu_wL, mu_wBL, mu_wBR,mu_d_holo;
    Mat holo_img, holo_img_depth;
    Mutex mu_holo,mu_pano_depth,mu_pano,mu_warped_pano;
    cuda::GpuMat warped_pano_gpu;
    Mat tmpH;
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
        dm.convertTo(dm, CV_64F);
        //dm = dm * f1.get_units();
        dm = dm / 1000.;
        return dm;
    }


};