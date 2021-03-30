#include "rsFeature.h"
#include <fstream>
#include <sstream>
#include <chrono>

rs2::pipeline rsFeature::getColorFrameRSpipeline() {
    rs2::pipeline pipe;
    rs2::config cfg;

    cfg.enable_stream(RS2_STREAM_COLOR, this->width, this->height, RS2_FORMAT_BGR8, 30);

    pipe.start(cfg);
    return pipe;
}
Mat rsFeature::getcolorFrameRS(rs2::pipeline pipe)
{

    Rect rct;
    int size = rct.width*rct.height * sizeof(Rect);
    SOCKET S;
    //send(S, (char*)&rct,size , 0);

    rs2::frameset frames;
    //for (int i = 0; i < 30; i++)
    //{
    //    frames = pipe.wait_for_frames();
    //}
    //while (true) {
     frames = pipe.wait_for_frames();
    rs2::frame color_frame = frames.get_color_frame();

    Mat color(Size(this->width, this->height), CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
    Mat frame = color.clone();
    return color;
    //namedWindow("Display Image", WINDOW_AUTOSIZE);
    //imshow("Display Image", color);
    
    //if(waitKey(1000/60)>=0) break;
    //}
    
}

rs2::pipeline rsFeature::setInfraredRS(int width, int height) {
    rs2::pipeline pipe;

    rs2::config cfg;
    this->width = width;
    this->height = height;
    cfg.enable_stream(RS2_STREAM_INFRARED, width, height, RS2_FORMAT_Y8, 15);
    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, 15);

    pipe.start(cfg);
    return pipe;
}
Mat rsFeature::getInfraredRS(rs2::pipeline pipe) {
   

     //frames;
    //for (int i = 0; i < 30; i++)
    //{
    //    frames = pipe.wait_for_frames();
    //}

    //while (true) {
     rs2::frameset frames = pipe.wait_for_frames();

        rs2::frame ir_frame = frames.first(RS2_STREAM_INFRARED);
        rs2::frame depth_frame = frames.get_depth_frame();

        Mat ir(Size(this->width, this->height), CV_8UC1, (void*)depth_frame.get_data(), Mat::AUTO_STEP);

        equalizeHist(ir, ir);
        applyColorMap(ir, ir, COLORMAP_JET);
        return ir;
        //namedWindow("Display Image", WINDOW_AUTOSIZE);
        //imshow("Display Image", ir);

        //if(waitKey(1)>=0) break;
    //}


}

void rsFeature::getDistance() {
    rs2_error* e = 0;
    rs2_context* ctx = rs2_create_context(RS2_API_VERSION, &e);
    check_error(e);

    rs2_device_list* device_list = rs2_query_devices(ctx, &e);
    check_error(e);
    int dev_count = rs2_get_device_count(device_list, &e);
    check_error(e);
    printf("There are %d connected RealSense devices.\n", dev_count);
    if (0 == dev_count)
        cout << "EXIT FAILURE" << endl;

    rs2_device* dev = rs2_create_device(device_list, 0, &e);
    rs2_pipeline* pipeline = rs2_create_pipeline(ctx, &e);
    check_error(e);

    rs2_config* config = rs2_create_config(&e);
    check_error(e);
    rs2_config_enable_stream(config, STREAM, STREAM_INDEX, WIDTH, HEIGHT, FORMAT, FPS, &e);
    check_error(e);



    rs2_pipeline_profile* pipeline_profile = rs2_pipeline_start_with_config(pipeline, config, &e);
    if (e)
    {
        printf("The connected device doesn't support depth streaming!\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // This call waits until a new composite_frame is available
        // composite_frame holds a set of frames. It is used to prevent frame drops
        // The returned object should be released with rs2_release_frame(...)
        rs2_frame* frames = rs2_pipeline_wait_for_frames(pipeline, RS2_DEFAULT_TIMEOUT, &e);
        check_error(e);

        // Returns the number of frames embedded within the composite frame
        int num_of_frames = rs2_embedded_frames_count(frames, &e);
        check_error(e);

        int i;
        for (i = 0; i < num_of_frames; ++i)
        {
            // The retunred object should be released with rs2_release_frame(...)
            rs2_frame* frame = rs2_extract_frame(frames, i, &e);
            check_error(e);

            // Check if the given frame can be extended to depth frame interface
            // Accept only depth frames and skip other frames
            if (0 == rs2_is_frame_extendable_to(frame, RS2_EXTENSION_DEPTH_FRAME, &e))
                continue;

            // Get the depth frame's dimensions
            int width = rs2_get_frame_width(frame, &e);
            check_error(e);
            int height = rs2_get_frame_height(frame, &e);
            check_error(e);

            // Query the distance from the camera to the object in the center of the image
            float dist_to_center = rs2_depth_frame_get_distance(frame, width / 2, height / 2, &e);
            check_error(e);

            // Print the distance
            printf("The camera is facing an object %.3f meters away.\n", dist_to_center);

            rs2_release_frame(frame);
        }

        rs2_release_frame(frames);
    }

    // Stop the pipeline streaming
    rs2_pipeline_stop(pipeline, &e);
    check_error(e);

    // Release resources
    rs2_delete_pipeline_profile(pipeline_profile);
    rs2_delete_config(config);
    rs2_delete_pipeline(pipeline);
    rs2_delete_device(dev);
    rs2_delete_device_list(device_list);
    rs2_delete_context(ctx);
}


void rsFeature::detectObject(Mat& img,Mat depthImage) {
    RNG rng(12345);
    Mat hsv;
    //convert RGB image into HSV image  
    cvtColor(depthImage, hsv, COLOR_BGR2HSV);
    //imshow("HSV",hsv);  
    Mat binary, binary1, imgToProcess;
    //get binary image  
    int thresh=18; //je geringer dieser Wert desto näher muss das Objekt sein, um es zu identifizieren
    inRange(hsv, Scalar(0, 85, 241), Scalar(thresh, 255, 255), binary);
     //imshow("Binary",binary);
     //waitKey(0);
    inRange(hsv, Scalar(171, 0, 0), Scalar(255, 255, 255), binary1);

    //binary.copyTo(binary1);
    add(binary1, binary, imgToProcess, noArray(),8);

    //absdiff(binary1, binary, imgToProcess);

    //find contours from binary image  
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(imgToProcess, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0)); //find contours  

    vector<vector<Point> > contours_poly(contours.size());
    vector<RotatedRect> minRect(contours.size());
    vector<RotatedRect> minEllipse(contours.size());
    vector<Rect> boundRect(contours.size());
    vector<float>radius(contours.size());
    vector<float>area(contours.size());
    vector<Point2f>center(contours.size());
    vector <int> areas;

    for (size_t i = 0; i < contours.size(); i++)
    {
        approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
        boundRect[i] = boundingRect(Mat(contours_poly[i]));
        minEnclosingCircle(contours_poly[i], center[i], radius[i]);
        area[i] = contourArea(Mat(contours_poly[i]));
    }

    int areaC = 0;
    int areaC2 = 0;
    int areaIdx = 0;
    int areaIdx2 = 0;
    Mat drawing = Mat::zeros(imgToProcess.size(), CV_8UC3);
    for (int i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        drawContours(drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
        ellipse(drawing, minEllipse[i], color, 2, 8);// ellipse

        if (boundRect[i].area() > 20000) {
            areas.push_back(i);
        }
            
        //if (boundRect[i].area() > areaC) {
        //    areaC = boundRect[i].area();
        //    areaIdx = i;
        //}
        //if (boundRect[i].area() > areaC2 && boundRect[i].area() < areaC) {
        //    areaC2 = boundRect[i].area();
        //    areaIdx2 = i;

        //}

        //rectangle(image, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
        //Point2f rect_points[4]; minRect[i].points( rect_points );
         //for( int j = 0; j < 4; j++ )
           //   line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );

    }
    for (int i = 0; i < areas.size(); i++) {
        rectangle(img, boundRect[areas[i]].tl(), boundRect[areas[i]].br(), (0, 0, 255), 2, 8, 0);

    }
    //rectangle(img, boundRect[areaIdx].tl(), boundRect[areaIdx].br(), (0, 0, 255), 2, 8, 0);
    //rectangle(img, boundRect[areaIdx2].tl(), boundRect[areaIdx2].br(), (0, 0, 255), 2, 8, 0);


}

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

void rsFeature::getdepth_and_colorFrame(bool isRunning,Mat &img1,Mat &depth_img) {
    rs2::pipeline pipe;
    rs2::colorizer color_map;
    rs2::config cfg;
    rs2::frameset frames;

    if (isRunning == false) {


        //Add desired streams to configuration
        cfg.enable_stream(RS2_STREAM_INFRARED, 640, 480, RS2_FORMAT_Y8, 15);
        cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 15);
        cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 15);

        //Instruct pipeline to start streaming with the requested configuration
        pipe.start(cfg);

        // Camera warmup - dropping several first frames to let auto-exposure stabilize
        for (int i = 0; i < 30; i++)
        {
            //Wait for all configured streams to produce a frame
            frames = pipe.wait_for_frames();
        }
        isRunning = true;
    }
    while (true) {
        frames = pipe.wait_for_frames();
        //rs2::frame ir_frame = frames.first(RS2_STREAM_INFRARED);
        rs2::frame depth_frame = frames.get_depth_frame().apply_filter(color_map);;
        //rs2::frameset frames1;
        rs2::frame color_frame = frames.get_color_frame();

        Mat color(Size(640, 480), CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
        //Mat ir(Size(640, 480), CV_8UC1, (void*)ir_frame.get_data(), Mat::AUTO_STEP);
        Mat depth(Size(640, 480), CV_8UC3, (void*)depth_frame.get_data(), Mat::AUTO_STEP);

        // Apply Histogram Equalization
        //equalizeHist(ir, ir);
        //applyColorMap(ir, ir, COLORMAP_JET);
        img1 = color;
        depth_img=depth;
        //// Display the image in GUI
        //namedWindow("Display Image", WINDOW_AUTOSIZE);
        //imshow("Display Image", ir);
        //imshow("color Image", color);
        //imshow("depth", depth);

        //if(waitKey(1)>=0) break;
    }
}

void rsFeature::RegionGrowth(Mat& src) {
    int min_region_area = int(min_region_area_factor * src.cols * src.rows);  // small region will be ignored
    cv::namedWindow("mask", WINDOW_NORMAL);

    // "dest" records all regions using different padding number
    // 0 - undetermined, 255 - ignored, other number - determined
    uchar padding = 1;  // use which number to pad in "dest"
    cv::Mat dest = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);

    // "mask" records current region, always use "1" for padding
    cv::Mat mask = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);

    // 4. traversal the whole image, apply "seed grow" in undetermined pixels
    for (int x = 0; x < src.cols; ++x) {
        for (int y = 0; y < src.rows; ++y) {
            if (dest.at<uchar>(cv::Point(x, y)) == 0) {
                grow(src, dest, mask, cv::Point(x, y), 200);

                int mask_area = (int)cv::sum(mask).val[0];  // calculate area of the region that we get in "seed grow"
                if (mask_area > min_region_area) {
                    dest = dest + mask * padding;   // record new region to "dest"
                    cv::imshow("mask", mask * 255);
                    cv::waitKey();
                    if (++padding > max_region_num) { printf("run out of max_region_num..."); }
                }
                else {
                    dest = dest + mask * 255;   // record as "ignored"
                }
                mask = mask - mask;     // zero mask, ready for next "seed grow"
            }
        }
    }
    printf("Mission complete...");
}

void rsFeature::grow(cv::Mat& src, cv::Mat& dest, cv::Mat& mask, cv::Point seed, int threshold) {
           
        
        /* apply "seed grow" in a given seed
         * Params:
         *   src: source image
         *   dest: a matrix records which pixels are determined/undtermined/ignored
         *   mask: a matrix records the region found in current "seed grow"
         */
        std::stack<cv::Point> point_stack;
        point_stack.push(seed);

        while (!point_stack.empty()) {
            cv::Point center = point_stack.top();
            mask.at<uchar>(center) = 1;
            point_stack.pop();

            for (int i = 0; i < 8; ++i) {
                cv::Point estimating_point = center + PointShift2D[i];
                if (estimating_point.x < 0
                    || estimating_point.x > src.cols - 1
                    || estimating_point.y < 0
                    || estimating_point.y > src.rows - 1) {
                    // estimating_point should not out of the range in image
                    continue;
                }
                else {
                    //                uchar delta = (uchar)abs(src.at<uchar>(center) - src.at<uchar>(estimating_point));
                                    // delta = (R-R')^2 + (G-G')^2 + (B-B')^2
                    int delta = int(pow(src.at<cv::Vec3b>(center)[0] - src.at<cv::Vec3b>(estimating_point)[0], 2)
                        + pow(src.at<cv::Vec3b>(center)[1] - src.at<cv::Vec3b>(estimating_point)[1], 2)
                        + pow(src.at<cv::Vec3b>(center)[2] - src.at<cv::Vec3b>(estimating_point)[2], 2));
                    if (dest.at<uchar>(estimating_point) == 0
                        && mask.at<uchar>(estimating_point) == 0
                        && delta < threshold) {
                        mask.at<uchar>(estimating_point) = 1;
                        point_stack.push(estimating_point);
                    }
                }
            }
        }
    }
void rsFeature::sendCamFrame(int port) {
    VideoCapture cap(0);
    server serv;
    SOCKET sock = serv.createSocket(port);
    Mat frame;
    cap.read(frame);
    while (true) {
        cap.read(frame);
        int frameSize = frame.total() * frame.elemSize();
        int sendframe = send(sock, (char*)frame.data, frameSize, 0);
        if (sendframe == -1) cout << "img send failed" << endl;
    }
    closesocket(sock);
}

void rsFeature::getWarpedImg(int port) {
    server serv;
    SOCKET sock = serv.createSocket(port);
    SOCKET sock2 = serv.createSocket(12000);
    //char buf[1843200];
    const int dataSize = 3686400;
    char* buf1 = new char[dataSize];
    char* buf2 = new char[10000];
    int bytes = 0;
    while (true) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

 
        Mat warp = Mat::zeros(Size(1280,960), CV_8UC3);

        int warpSize = warp.total() * warp.elemSize();
        ZeroMemory(buf1, warpSize);
        for (int i = 0; i < warpSize; i += bytes)
            if ((bytes = recv(sock, buf1+i , warpSize-i, 0)) == -1) cout << ("recv failed");
        int ptr = 0;
        for (int i = 0; i < warp.rows; i++) {
            for (int j = 0; j < warp.cols; j++) {
                warp.at<Vec3b>(i, j) = Vec3b(buf1[ptr + 0], buf1[ptr + 1], buf1[ptr + 2]);
                ptr = ptr + 3;
            }
        }
        std::chrono::steady_clock::time_point threadtime = std::chrono::steady_clock::now();
        std::cout << "fpms = " << std::chrono::duration_cast<std::chrono::microseconds>(threadtime - begin).count() << "[µs]" << std::endl;
        imshow("warp", warp);
        if (waitKey(1000 / 60) >= 0) break;
    }
    closesocket(sock);
}