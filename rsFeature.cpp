#include "rsFeature.h"
#include <fstream>
#include <sstream>
#include <chrono>



void rsFeature::reference_cam(String deviceNumber) {
    rs2::pipeline pipe;
    rs2::colorizer color_map;
    rs2::config cfg;
    rs2::frameset frames;
    rs2::context ctx;
    sock socket1;
    //Multicast-Socket für das Senden des Referenzbildes
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
            cout << "can't get frames on camera: " << deviceNumber << endl;
        }
    }


    while (true)
    {

        frames = pipe.wait_for_frames();
        this->m_img = frame_to_mat(frames.get_color_frame()).clone();

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
        this->m_holo = frame_to_mat(frames.get_color_frame());
        mu_holo.unlock();
        mu_d_holo.lock();
        this->m_holo_d = depth_frame_to_meters(frames.get_depth_frame());
        mu_d_holo.unlock();
    }
}


void rsFeature::sichteinschränkungsdetektion(sock socket) {
    socket.initMultiCastUDP(54221, "192.168.100.34", "226.1.1.2");
    bool verdeckung_bool = false;

    while (true) {
        if (this->m_d_img.empty()) continue;
        auto start = std::chrono::system_clock::now(); //Zeitmessung falls erforderlich

        mu_dimg.lock();
        Mat D = this->m_d_img.clone();  //Speichern des Tiefenbildes der Referenzkamera
        mu_dimg.unlock();

        Mat depth_img = getMatinRange(D, 0.1, 1.); //Bereich zwischen 0.1m bis 1m extrahieren
        depth_img.convertTo(depth_img, CV_32FC3, 1.0 / 255); //Pixelwerte normieren auf Werte zwischen 0 und 1

        Mat depth_img_up = depth_img(Rect(0, 0, depth_img.cols, depth_img.rows / 2)); //Oberen Bereich in Mat File abspeichern

        double verdeckung_gesamt = sum(depth_img)[0] / (255 * depth_img.cols * depth_img.rows); //Pixelwerte inkrementieren und durch Bildbreite-, höhe und maximalen Pixelwert teilen
        double verdeckung_Oben = sum(depth_img_up)[0] / (depth_img_up.cols * depth_img_up.rows * 255); //Pixelwerte für obere Hälfte des Bildes inkrementieren und durch Bildbreite-, höhe und maximalen Pixelwert teilen
       
        if (verdeckung_gesamt > 0.34 || verdeckung_Oben > 0.20) verdeckung_bool = true; //0.34 oder 0.2 ist der Schwellwert ab dem den Clients signalisiert wird, dass die Referenzkamera verdeckt wird. Kann variiert werden
        else verdeckung_bool = false;
        socket.send_hindernis(verdeckung_bool);


        auto end = std::chrono::system_clock::now(); // Ende der Zeitmessung
        auto elapsed = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }
}

/*Erhalten der Bilder.Hier muss für jeden Client ein einzelnes Socket erstellt werden. 
  Diese Funktion wird in der main in vier verschiedenen Threads ausgeführt. */
void rsFeature::recv_img(int port, string IP) {
    sock socket1;
    socket1.initUDP(port, IP);
    while (true) {

        Mat img;
        
        socket1.rcv_img(img); //Empfangen des Bildes
        if (img.empty()) continue;
        //Zuordnung der Kamerabilder nach Portnummer. Wichtig hierbei sind die Mutexe wie z.b mu_wL (mutex warp Left)

        if (port == 55000) {
            mu_wL.lock();
            this->m_imgL = img.clone(); //left frame
            mu_wL.unlock();
        }
        else if (port == 55000 + 2) {
            mu_wBR.lock();
            this->m_imgBR = img.clone(); //bottom right frame
            mu_wBR.unlock();

        }
        else if (port == 55000 + 3) {
            mu_wR.lock();
            this->m_imgR = img.clone(); //right frame
            mu_wR.unlock();

        }
        else if (port == 55000 + 4) {
            mu_wBL.lock();
            this->m_imgBL = img.clone(); //bottom left frame
            mu_wBL.unlock();

        }
    }
    socket1.closeSock();
}

//Funktion um Detektionsbereich zu extrahieren
Mat rsFeature::getMatinRange(Mat depth_img, double minRange, double maxRange) {
    cuda::GpuMat A, B, gpu_res, gpu_dimg;
    Mat res;

    gpu_dimg.upload(depth_img);
    cuda::threshold(gpu_dimg, A, minRange, 255, THRESH_BINARY);
    cuda::threshold(gpu_dimg, B, maxRange, 255, THRESH_BINARY);
    cuda::absdiff(A, B, gpu_res);

    gpu_res.download(res);
    return res;
}


