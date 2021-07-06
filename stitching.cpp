#include "stitching.h"


//Stitching::Stitching() {
//	Mat imgLeft;
//	Mat imgRight;
//	Mat imgCenter;
//	Mat imgBottom1;
//	Mat imgBottom2;
//
//	this->imgLeft = imgLeft;
//	this->imgRight = imgRight;
//	this->imgCenter = imgCenter;
//	this->imgBottom1 = imgBottom1;
//	this->imgBottom2 = imgBottom2;
//
//}





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
			//cout << i << " area  " << a << endl;
			// Store the index of largest contour
			largest_contour_index = i;
			// Find the bounding rectangle for biggest contour
			rct = boundingRect(contours[largest_contour_index]);
		}
	}
	return rct;
}


Mat Stitching::makeNormalize(Mat img) {
	Mat grayImg, bin,dst;
	//if (img.type() != 0) {
	//	cvtColor(img, grayImg, COLOR_BGR2GRAY);
	//}
	//else {
	//	grayImg = img;
	//}
	cvtColor(img, grayImg, COLOR_RGB2GRAY);


	//removeBlackPoints(grayImg);

	cuda::GpuMat grayImgGPU, binGPU, dstGPU;
	//grayImgGPU.upload(grayImg);
	//cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);
	threshold(grayImg, bin, 0, 255, THRESH_BINARY);

	
	distanceTransform(bin, dst, DIST_L2, 3.0);
	cv::normalize(dst, dst, 0., 1., cv::NORM_MINMAX);
	return dst;

}

void Stitching::streamRealSense()
{
	rs2::context ctx;        // Create librealsense context for managing devices

	std::map<std::string, rs2::colorizer> colorizers; // Declare map from device serial number to colorizer (utility class to convert depth data RGB colorspace)
	rs2::colorizer color_map, color_map2, color_map3, color_map4, color_map5, color_map6;


	// Capture serial numbers before opening streaming
	vector<rs2::pipeline> pipe_vec;

	std::vector<std::string>              serials;
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
		rs2::pipeline p;
		pipe_vec.push_back(p);
	}


	rs2::pipeline pipe(ctx), pipe2(ctx), pipe3(ctx), pipe4(ctx), pipe5(ctx), pipe6(ctx);
	rs2::config cfg, cfg2, cfg3, cfg4, cfg5,cfg6;
	vector<rs2::config> cfg_vec;
	vector<rs2::frame> frame_vec;
	vector<rs2::frame> frame_depth_vec;
	int width = 640;
	int height = 480;
	int fps = 15;
	for (int i = 0; i < serials.size(); i++) {
		cfg.enable_device(serials[i]);
		cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, fps);
		cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
		pipe_vec[i].start(cfg);
		rs2::frame color_frame0;
		rs2::frame depth_frame0;

		frame_vec.push_back(color_frame0);
		frame_depth_vec.push_back(depth_frame0);

	}
	
	std::map<int, rs2::frame> render_frames;
	rs2::frameset framesOCV;

	vector<VideoWriter> vidRGB,vidDepth;
	for (int i = 0; i < serials.size(); i++) {
		int k = i;
		VideoWriter video("C:\\Users\\Aufnahmen\\cam" + to_string(i) + "\\color\\cam" + to_string(i) + "d.tiff", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(width, height));
		vidRGB.push_back(video);
	}
	for (int i = 0; i < serials.size(); i++) {
		VideoWriter videod0("C:\\Users\\Aufnahmen\\cam" + to_string(i) + "\\depth\\cam"+to_string(i)+"d.tiff", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(width, height));
		vidDepth.push_back(videod0);
	}
	
	
	rs2::frameset frames0, frames1, frames2, frames3, frames4, frames5;
	rs2::frame depth_frame0, depth_frame1, depth_frame2, depth_frame3, depth_frame4, depth_frame5;
	rs2::frameset fm;
	for (int i = 0; i < 30; i++)
	{

		for (int j = 0; j < frame_vec.size(); j++) {
			fm = pipe_vec[j].wait_for_frames();
		}

	}


	while (true) {	
		vector<Mat> imgs;
		vector<Mat> dimgs;
		for (int j = 0; j < frame_vec.size(); j++) {
			fm = pipe_vec[j].wait_for_frames();
			frame_vec[j] = fm.get_color_frame().apply_filter(color_map);
			frame_depth_vec[j] = fm.get_depth_frame();
			Mat img = frame_to_mat(frame_vec[j]);
			Mat dimg= depth_frame_to_meters(frame_depth_vec[j]);
			dimgs.push_back(dimg);
			imgs.push_back(img);
		}
		
		for (int i = 0; i < vidRGB.size(); i++) {
			vidRGB[i].write(imgs[i]);
		}


		for (int i = 0; i < vidDepth.size(); i++) {
			vidDepth[i].write(dimgs[i]);
		}

	}

}

cuda::GpuMat Stitching::getAlphaGPU(cuda::GpuMat gpu_dst1, cuda::GpuMat gpu_dst2) {
	cuda::GpuMat gpu_alpha(gpu_dst1.size(), CV_32F);
	cuda::GpuMat added, divalpha, resAlpha;
	//gpu_dst1.upload(dst1);
	//gpu_dst2.upload(dst2);
	//gpu_alpha.upload(alpha);
	cuda::add(gpu_dst1, gpu_dst2, added);
	cuda::divide(gpu_dst1, added, resAlpha,2.);
	return resAlpha;


}

void Stitching::makeNormalizethread(cuda::GpuMat img, cuda::GpuMat &dst_gpu) {
	Mat grayImg, bin,dst;

	cuda::GpuMat grayImgGPU, binGPU, dstGPU,img_GPU, dst_gpu_tmp;

	//img_GPU.upload(img);

	cuda::cvtColor(img, grayImgGPU, COLOR_BGR2GRAY);

	cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);

	binGPU.download(bin);

	distanceTransform(bin, dst, DIST_L2, 3.0);
	dst_gpu_tmp.upload(dst);
	cuda::normalize(dst_gpu_tmp, dst_gpu, 0., 1., NORM_MINMAX,CV_32F);

}

void Stitching::AlphaBlendingGPU(Mat img, Mat addImg,Mat &Pano) {
	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	Stitching st;
	cuda::GpuMat gpu_alpha[3], gpu_beta[3], gpu_alphares, gpu_betares, alphtmp, betatmp;
	cuda::GpuMat alpha, beta;
	cuda::GpuMat dst1, dst2,dstDiff;
	cuda::GpuMat gpu_img1, gpu_img2;

	gpu_img1.upload(img);

	gpu_img2.upload(addImg);
	thread t1(makeNormalizethread, gpu_img1, ref(dst1));
	thread t2(makeNormalizethread, gpu_img2, ref(dst2));
	t1.join();
	t2.join();
	//cuda::bitwise_and(dst1, dst2, dstDiff);
	//cuda::absdiff(dst2, dstDiff, dst2);

	alpha = st.getAlphaGPU(dst1, dst2);
	beta = cuda::GpuMat(alpha.size(), alpha.type(), Scalar::all(1));

	vector<cuda::GpuMat> vecgpu1, vecgpu2;
	cuda::absdiff(alpha, beta, betatmp);


	//Mat Pano;
	cuda::GpuMat pano2, panothresh, panodiff;

	cuda::blendLinear(gpu_img1, gpu_img2, alpha, betatmp, panodiff);

	panodiff.download(Pano);
	gpu_alpha->release();
	gpu_beta->release();

}