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
    //Membervariablen
    Mat m_img; //Membervariable für die Referenzkamera-> wird in void reference_cam(String deviceNumber) intialisert
    Mat m_d_img; //Membervariable für das Tiefenbild der Referenzkamera -> wird in void reference_cam(String deviceNumber) intialisert
    Mat m_pano; //Membervariable für die rekonstruierte Szene. Wird in der main in scene_reconstruction initialisiert
    Mat m_imgR, m_imgL, m_imgBL, m_imgBR; //Membervariablen für die Hubmast- und Gabelzinkenkameras
    Mat m_holo, m_holo_d; //Membervariablen für sichtfeldkamera rgb und das entsprechende Tiefenbild
    Mutex mu_wR, mu_wL, mu_wBL, mu_wBR; //Mutexes für die Umgebungskameras, werden in recv_img() und scene_reconstruction() verwendet
    Mutex mu_d_holo, mu_holo, mu_dimg; //Mutexes für Sichtfeldkameras sowie das entsprechende Tiefenbild, wird in der main und void holo_cam verwendet
    Mutex mu_pano; //Mutex für die rekonstruierte Szene, da in der main in zwei verschiedenen Threads darauf zugegriffen wird: scene_reconstruction und sichteinschränkungskompensation 

    //Methoden
    void sichteinschränkungsdetektion(sock socket); 
    void reference_cam(String deviceNumber);
    void holo_cam(String deviceNumber);
    void recv_img(int port, string IP);
    Mat getMatinRange(Mat depth_img, double minRange = 0.3, double maxRange = 1.5);

    //Funktion um RGB Bilder von Realsense in OpenCV Format zu überführen
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


    //Funktion um Tiefenbilder von Realsense in OpenCV Format zu überführen
    static cv::Mat depth_frame_to_meters(const rs2::depth_frame& f1)
    {

        cv::Mat dm = frame_to_mat(f1);
        dm.convertTo(dm, CV_64F);
        //dm = dm * f1.get_units();
        dm = dm / 1000.;
        return dm;
    }


};