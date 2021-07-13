#include "stitching.h"



/*statische Funktion, um Distanztransformation eines Bildes zu ermitteln */
void Stitching::makeNormalizethread(cuda::GpuMat img, cuda::GpuMat& dst_gpu) {
	Mat grayImg, bin, dst;
	cuda::GpuMat grayImgGPU, binGPU, dst_gpu_tmp;

	//Bild in Grauwert konvertieren
	cuda::cvtColor(img, grayImgGPU, COLOR_BGR2GRAY);

	//Grauwertbild komplett weiß machen
	cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);
	//Bild auf CPU herunterladen, da keine GPU-basierte Funktion für die Distanztransformation existiert
	binGPU.download(bin);
	//Distanztransformation mittels L2-Norm
	distanceTransform(bin, dst, DIST_L2, 3.0);
	//Distanztransformiertes Bild auf CPU übertragen
	dst_gpu_tmp.upload(dst);
	//GPU-Basierte Normalisierung des Bildes. Wird für die Ermittlung der Alpha-Matrix benötigt
	cuda::normalize(dst_gpu_tmp, dst_gpu, 0., 1., NORM_MINMAX, CV_32F);

}

/*Funktion, um Helligkeitsintensitäten an den entsprechenden Pixelkoordinaten auszurechnen (GPU basiert)-> (dst1+dst2)/dst1																										
Bekommt als Inputparameter die normalisierten Distanztransformationen die in makenormalizethread(...) ermittelt werden. 
*/
cuda::GpuMat Stitching::getAlphaGPU(cuda::GpuMat gpu_dst1, cuda::GpuMat gpu_dst2) 
{

	cuda::GpuMat added, divalpha, resAlpha;
	cuda::add(gpu_dst1, gpu_dst2, added);
	cuda::divide(gpu_dst1, added, resAlpha, 2.);
	return resAlpha;
}


void Stitching::AlphaBlendingGPU(Mat img, Mat addImg, Mat& Pano) {
	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	Stitching st;
	cuda::GpuMat alpha, beta, betatmp, gpu_pano;
	cuda::GpuMat dst1, dst2, dstDiff;
	cuda::GpuMat gpu_img1, gpu_img2;

	// Zu stitchende Bilder auf GPU hochladen
	 gpu_img1.upload(img);
	 gpu_img2.upload(addImg);
	
	 // Distanztransformation von zu stitchenden Bilder und anschließend normalisieren
	thread t1(makeNormalizethread, gpu_img1, ref(dst1));
	thread t2(makeNormalizethread, gpu_img2, ref(dst2));
	t1.join();
	t2.join();

	// Helligkeitsintensitäten des ersten Bildes img bzw. gpu_img1 an Pixelkoordinaten ermitteln -> Alpha Matrix A(x,y)
	alpha = st.getAlphaGPU(dst1, dst2);

	/* Helligkeitsintensitäten des zweiten Bildes addImg bzw.gpu_img2 an Pixelkoordinaten ermitteln->Beta Matrix B(x, y) = 1 - A(x, y)
	Dafür wird zunächst ein Bild mit 1en erzeugt (betatmp). Anschließend wird die Differenz zwischen alpha und betatmp ermittelt, woraus die Beta
	Matrix resultiert */ 
	betatmp = cuda::GpuMat(alpha.size(), alpha.type(), Scalar::all(1));
	cuda::absdiff(alpha, betatmp, beta);

	//Lineares Alpha-Blending. Panorama wird in gpu_pano abgespeichert.
	cuda::blendLinear(gpu_img1, gpu_img2, alpha, beta, gpu_pano);

	//Panorama von GPU auf CPU übertragen
	gpu_pano.download(Pano);

}