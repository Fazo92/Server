#include "stitching.h"
#include "rsFeature.h"
#include "stitching.h"
#include "sock.h"

#define IPJETSON0 "192.168.100.110"
#define IPJETSON2 "192.168.100.162"
#define IPJETSON3 "192.168.100.4"
#define IPJETSON4 "192.168.100.176"
#define PORTSENDIMG 54000
#define PORTRCVIMG 55000
#define PORTRCVDIMG 56000
#define PORTHINDERNIS 53000

typedef float  Float32;
typedef long float  Float64;
using namespace cv::cuda;
typedef Point3_<uint8_t> Pixel;
void showImage(Mat img) {
	while (true) {
		if (img.empty()) continue;
		imshow("img", img);
		if (waitKey(1000 / 60) >= 0) break;
	}

}

void scene_reconstruction(rsFeature& rsf, Stitching& st) {
	while (true) {

		//if (rsf.m_img.empty()) cout << "center Image is empty" << endl;
		//if (rsf.m_imgR.empty()) cout << "Left Image is empty" << endl;
		//if (rsf.m_imgL.empty()) cout << "Right Image is empty" << endl;
		//if (rsf.m_imgBR.empty()) cout << "Bottom Left Image is empty" << endl;
		//if (rsf.m_imgBL.empty()) cout << "Bottom Right Image is empty" << endl;

		if (rsf.m_imgR.empty() || rsf.m_imgL.empty() || rsf.m_imgBR.empty() || rsf.m_imgBL.empty()) continue;
		//// Speichern der transformierten Bilder
		rsf.mu_wR.lock();
		Mat warpR = rsf.m_imgR.clone();
		rsf.mu_wR.unlock();

		rsf.mu_wL.lock();
		Mat warpL = rsf.m_imgL.clone();
		rsf.mu_wL.unlock();

		rsf.mu_wBR.lock();
		Mat warpBR = rsf.m_imgBR.clone();
		rsf.mu_wBR.unlock();
		rsf.mu_wBL.lock();
		Mat warpBL = rsf.m_imgBL.clone();
		rsf.mu_wBL.unlock();

		//// Stitchen der Teilbilder mittels Alpha-Blending
		Mat pano, pano2, pano3;
		thread tP1(&Stitching::AlphaBlendingGPU, warpR, warpL, ref(pano));
		thread tP2(&Stitching::AlphaBlendingGPU, warpBL, warpBR, ref(pano2));
		tP1.join();
		tP2.join();
		thread tP3(&Stitching::AlphaBlendingGPU, pano2, pano, ref(pano3));
		tP3.join();

		//// rekonstruierte Szene speichern
		rsf.mu_pano.lock();
		rsf.m_pano = pano3.clone();
		rsf.mu_pano.unlock();

	}
}
void sichteinschränkungs_komp(rsFeature& rsf)
{
	///Wert für Ausblenden der Sichteinschränkungen. Je höher, desto stärker werden die Sichteinschränkungen ausgeblendet
	float alpha = 0.7;

	while (true)
	{
		if (rsf.m_holo.empty() || rsf.m_holo_d.empty() || rsf.m_pano.empty()) continue;
		Mat sichtfeld, sichtfeld_depth, pano, pano_prev;

		//// rekonstruierte Szene mit und ohne eingeblendetem Sichtfeld speichern
		rsf.mu_pano.lock();
		pano = rsf.m_pano.clone();
		pano_prev = rsf.m_pano.clone()(Rect(rsf.m_holo.cols / 2, rsf.m_holo.rows / 2, rsf.m_holo.cols, rsf.m_holo.rows));
		rsf.mu_pano.unlock();
		Mat roi(pano, Rect(rsf.m_holo.cols / 2, rsf.m_holo.rows / 2, rsf.m_holo.cols, rsf.m_holo.rows));
		rsf.mu_holo.lock();
		sichtfeld = rsf.m_holo.clone();
		rsf.mu_holo.unlock();
		sichtfeld.copyTo(roi);

		////Tiefenbild der Sichtfeldkamera 
		rsf.mu_d_holo.lock();
		sichtfeld_depth = rsf.m_holo_d.clone();
		rsf.mu_d_holo.unlock();

		////Bereich von 0.28 bis 3 metern extrahieren. Wird im Anschluss als Maske verwendet
		Mat mask = rsf.getMatinRange(sichtfeld_depth, 0.28, 3.);

		/// Ausschnitt der rekonstruierten Szene in Bild speichern, um es anschließend zu verarbeiten
		Mat croppedPano = pano(Rect(rsf.m_holo.cols / 2, rsf.m_holo.rows / 2, rsf.m_holo.cols, rsf.m_holo.rows));

		/// parallales verarbeiten der Pixel der rekonstruierten Szene
		croppedPano.forEach<Pixel>(
			[&sichtfeld, &pano_prev, &mask, &alpha](Pixel& pixel, const int* position)->void
			{
				// SichtfeldKamera =px_SF
				Pixel px_SF = sichtfeld.ptr<Pixel>(position[0])[position[1]];
				// Ausschnitt aus rekonstruierte Szene ohne Sichtfeld =px_pano
				Pixel px_pano = pano_prev.ptr<Pixel>(position[0])[position[1]];
				// Maske die vorher aus Tiefenbildern der Sichtfeldkamera gewonnen wurden
				double val = mask.ptr<double>(position[0])[position[1]];
				/// Wenn Pixelintensität ungleich Null ist, wird ein transparents Überlagern mit dem Wert alpha durchgeführt
				if (val != 0.f) {
					pixel.x = int(px_pano.x) * alpha + (int)px_SF.x * (1 - alpha);
					pixel.y = int(px_pano.y) * alpha + (int)px_SF.y * (1 - alpha);
					pixel.z = int(px_pano.z) * alpha + (int)px_SF.z * (1 - alpha);

				}
			}
		);
		imshow("final", pano);
		waitKey(1);
	}
}




void main() {
	Stitching st;
	rsFeature rsf;


	sock socket1, socket2, socket3, socket4, socketbool;
	//cuda::setDevice(0);
	thread timg_C(&rsFeature::reference_cam, &rsf, "042222071631");
	thread timg_Holo(&rsFeature::holo_cam, &rsf, "801312070973");
	thread tcomputeHindernis(&rsFeature::sichteinschränkungsdetektion, &rsf, socketbool);
	thread trcvImg0(&rsFeature::recv_img, &rsf, PORTRCVIMG, IPJETSON0);
	thread trcvImg2(&rsFeature::recv_img, &rsf, PORTRCVIMG + 2, IPJETSON2);
	thread trcvImg3(&rsFeature::recv_img, &rsf, PORTRCVIMG + 3, IPJETSON3);
	thread trcvImg4(&rsFeature::recv_img, &rsf, PORTRCVIMG + 4, IPJETSON4);

	thread tstitch(scene_reconstruction, ref(rsf), ref(st));
	thread tstitch2(sichteinschränkungs_komp, ref(rsf));

	timg_C.join();


	return;
}