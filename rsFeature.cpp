#include "rsFeature.h"
#include <fstream>
#include <sstream>
#include <chrono>



Mat rsFeature::getDepthFrame(rs2::pipeline pipe, rs2::colorizer color_map) {

    rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
    rs2::frame depth = data.get_depth_frame().apply_filter(color_map);

    // Query frame size (width and height)
    const int w = depth.as<rs2::video_frame>().get_width();
    const int h = depth.as<rs2::video_frame>().get_height();
    this->width = w;
    this->height = h;
    // Create OpenCV matrix of size (w,h) from the colorized depth data
    Mat image(Size(w, h), CV_8UC3, (void*)depth.get_data(), Mat::AUTO_STEP);
    return image;
}

void rsFeature::reference_cam(String deviceNumber) {
    rs2::pipeline pipe;
    rs2::colorizer color_map;
    rs2::config cfg;
    rs2::frameset frames;
    rs2::context ctx;
    sock socket1;
    socket1.initMultiCastUDP(54222, "192.168.100.34", "226.1.1.1");


    cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
       
    cfg.enable_device(deviceNumber);
    rs2::device dev = ctx.query_devices()[0];
    pipe.start(cfg);
    for (int i = 0; i < 30; i++)
    {
        try 
        {
           frames = pipe.wait_for_frames();
        }
        catch (exception& e) {
           cout << "can't get frames on camera: " << deviceNumber<<endl;
         }
     }


    while (true) 
    {

        frames = pipe.wait_for_frames();
            mu_img_send.lock();
            this->m_img = frame_to_mat(frames.get_color_frame()).clone();
            mu_img_send.unlock();

            Mat k = depth_frame_to_meters(frames.get_depth_frame());
            mu_dimg.lock();
            this->m_d_img = k.clone();
            mu_dimg.unlock();
            socket1.send_img(m_img);

    }
    socket1.closeSock();

}


void rsFeature::holo_cam(String deviceNumber) {
    rs2::pipeline pipe;
    rs2::colorizer color_map;
    rs2::config cfg;
    rs2::frameset frames;
    rs2::context ctx;

    cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);

    cfg.enable_device(deviceNumber);
    rs2::device dev = ctx.query_devices()[0];
    pipe.start(cfg);
    for (int i = 0; i < 30; i++)
    {
        try
        {
            frames = pipe.wait_for_frames();
        }
        catch (exception& e) {
            cout << "can't get frames on camera: " << deviceNumber << endl;
        }
    }


    while (true) {

        frames = pipe.wait_for_frames();     
        mu_holo.lock();
        this->holo_img = frame_to_mat(frames.get_color_frame());
        mu_holo.unlock();
        mu_d_holo.lock();
        this->holo_img_depth = depth_frame_to_meters(frames.get_depth_frame());
        mu_d_holo.unlock();
    }
}


void rsFeature::sichteinschränkungsdetektion(sock socket) {
    socket.initMultiCastUDP(54221, "192.168.100.34", "226.1.1.2");

    while (true) {
        if (this->m_d_img.empty()) continue;
        auto start = std::chrono::system_clock::now();

        mu_dimg.lock();
        Mat D = this->m_d_img.clone();
        mu_dimg.unlock();
        Mat depth_img = getMatinRange(D, 0.1, 1.);
        mu_d_img_Range.lock();
        d_img_Range = depth_img.clone();
        d_img_Range.convertTo(d_img_Range, CV_32FC3, 1.0 / 255);

        mu_d_img_Range.unlock();
        Mat depth_img_up = depth_img(Rect(0, 0, depth_img.cols, depth_img.rows / 2));
        this->hindernisVal = sum(depth_img)[0] / (255 * depth_img.cols * m_d_img.rows);
        double hindernisUp = sum(depth_img_up)[0] / (depth_img_up.cols * depth_img_up.rows * 255);
        if (hindernisVal > 0.34 || hindernisUp > 0.20) this->hindernis = true;
        else this->hindernis = false;
        socket.send_hindernis(this->hindernis);
        auto end = std::chrono::system_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }
}

void rsFeature::recv_img(int port, string IP) {
    sock socket1;
    socket1.initUDP(port,IP);
    while (true) {

        Mat img;
        socket1.rcv_img(img);
        if (img.empty()) continue;

        if (port == 55000) {
            mu_wL.lock();
            this->m_imgL=img.clone();
            mu_wL.unlock();
        }
        else if (port == 55000 + 2) {
            mu_wBR.lock();
               this->m_imgBR=img.clone();
            mu_wBR.unlock();

            }
        else if (port == 55000 + 3) {
            mu_wR.lock();
            this->m_imgR = img.clone();
            mu_wR.unlock();

        }
        else if (port == 55000 + 4) {
            mu_wBL.lock();
            this->m_imgBL = img.clone();
            mu_wBL.unlock();

        }
        }
    socket1.closeSock();
    }

Mat rsFeature::getMatinRange(Mat depth_img, double minRange , double maxRange) {
    cuda::GpuMat A, B, gpu_res,gpu_dimg;
    Mat res;

    gpu_dimg.upload(depth_img);
   cuda::threshold(gpu_dimg, A, minRange, 255, THRESH_BINARY);
    cuda::threshold(gpu_dimg, B, maxRange, 255, THRESH_BINARY);
    cuda::absdiff(A, B, gpu_res);
    
    gpu_res.download(res);
    return res;
}


