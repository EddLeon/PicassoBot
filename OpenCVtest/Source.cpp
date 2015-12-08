#include<iostream>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "Serial.h"


using namespace cv;
using namespace std;

SerialEmul *Serial;


int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

Mat src;
Mat src_gray;
RNG rng(12345);
Mat imagenThreshold;
Mat canny_output;
int thresh = 100;
int max_thresh = 255;
Mat HSV;
Mat thresholdIm;
int renglones;
int columnas;

vector<Point> resumen;
vector<Point> resumen2;
Size tam;

void thresh_callback(Mat & mat);
void sendPoints(vector<Point> puntos);  // envio serial de puntos al arduino

void drawLines3(vector<Point> v) {
	Mat resumida = Mat::zeros(30, 30, CV_8UC3);
	for (int i = 0; i< v.size() - 1; i++)
	{
		Scalar color = Scalar(rng.uniform(5, 255), rng.uniform(5, 255), rng.uniform(5, 255));

		line(resumida, v[i], v[i + 1], color, 1, 8, 0);
	}

	imshow("Segundo resumen en escala", resumida);
}

void drawLines(vector<Point> v) {
	Mat resumen1 = Mat::zeros(src.size(), CV_8UC3);
	for (int i = 0; i< v.size() - 1; i++)
	{
		Scalar color = Scalar(rng.uniform(5, 255), rng.uniform(5, 255), rng.uniform(5, 255));

		line(resumen1, v[i], v[i + 1], color, 1, 8, 0);
	}

	imshow("Primer resumen", resumen1);
}

void resume(vector<Point> puntos) {
	int comp1 = puntos[0].x;
	int comp2 = puntos[0].y;
	int j = 0;
	resumen.push_back(Point(puntos[0].x, puntos[0].y));

	for (int i = 1; i < puntos.size(); i++) {
		if ((abs(comp1 - puntos[i].x) > 10) || (abs(comp2 - puntos[i].y) > 10)) {
			resumen.push_back(Point(puntos[i].x, puntos[i].y));
			comp1 = puntos[i].x;
			comp2 = puntos[i].y;
			j++;
		}
		if (i > 10) {
			if (abs(resumen[0].x - puntos[i].x) < 10 && abs(resumen[0].y - puntos[i].y) < 10) {
				resumen.push_back(resumen[0]);
				break;
			}
		}
	}
}

void resume2(vector<Point> puntos) {
	int comp1 = puntos[0].x;
	int comp2 = puntos[0].y;
	int j = 0;
	resumen2.push_back(Point(puntos[0].x, puntos[0].y));

	for (int i = 1; i < puntos.size(); i++) {
		if ((abs(comp1 - puntos[i].x) > 4) || (abs(comp2 - puntos[i].y) > 4)) {
			resumen2.push_back(Point(puntos[i].x, puntos[i].y));
			comp1 = puntos[i].x;
			comp2 = puntos[i].y;
			j++;
		}
		if (i > 10) {
			if (abs(resumen2[0].x - puntos[i].x) < 4 && abs(resumen2[0].y - puntos[i].y) < 4) {
				resumen2.push_back(resumen2[0]);
				break;
			}
		}
	}
}

vector<Point> escala(vector<Point> resumido) {
	vector<Point> escalado;
	for (int i = 0; i< resumido.size(); i++) { //entre el valor de la escala
		escalado.push_back(Point(resumido[i].x /8 , resumido[i].y / 8));
	}

	return escalado;
}


//default capture width and height
const int FRAME_WIDTH = 200;
const int FRAME_HEIGHT = 200;

//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string trackbarWindowName = "Trackbars";

void on_trackbar(int, void*)
{
	cout << "Hmin: " << H_MIN << endl;
	cout << "Hmax: " << H_MAX << endl;
	cout << "Smin: " << S_MIN << endl;
	cout << "Smax: " << S_MAX << endl;
	cout << "Vmin: " << V_MIN << endl;
	cout << "Vmax: " << V_MAX << endl;
}

string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}


void createTrackbars() {
	//create window for trackbars
	namedWindow(trackbarWindowName, 0);


	//create memory to store trackbar name on window
	char TrackbarName[50];
	printf(TrackbarName, "H_MIN %d", H_MIN);
	printf(TrackbarName, "H_MAX %d", H_MAX);
	printf(TrackbarName, "S_MIN %d", S_MIN);
	printf(TrackbarName, "S_MAX %d", S_MAX);
	printf(TrackbarName, "V_MIN %d", V_MIN);
	printf(TrackbarName, "V_MAX %d", V_MAX);

	//create trackbars and insert them into window
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}


int main(int argc, char* argv[])
{
	//createTrackbars();

	
	//Conecting to serial device
	Serial = new SerialEmul();
	int port = -1;
	char c_baud;
	int baud;

	while (port<1 || port>99) {
		cout << "*Enter Port # : ";
		cin >> port;
	}
	cout << endl;
	cout << "*Select Baud Rate :  \n-(default 9600)" << endl;
	cout << "a) 4800\nb) 9600\nc) 14400\nd) 19200\ne) 28800\nf) 38400\ng) 57600\nh) 115200" << endl;
	cin >> c_baud;

	switch (c_baud) {
	case 'a': baud = 4800; break;
	case 'b': baud = 9600; break;
	case 'c': baud = 14400; break;
	case 'd': baud = 19200; break;
	case 'e': baud = 28800; break;
	case 'f': baud = 38400; break;
	case 'g': baud = 57600; break;
	case 'h': baud = 115200; break;
	default: baud = 9600; break;
	}
	cout << "-------------------------------" << endl;
	if (!Serial->Open(port, baud)) {
		cout << "Error: Could not connect to COM" << port << endl;
		exit(-1l);
	}
	

	VideoCapture capture;
	capture.open(1);

	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	renglones = src.rows;
	columnas = src.cols;

	bool stop = true;
	char k=0;
	while (k!=27) {

		capture.read(src);

		//--cvtColor(src, HSV, COLOR_BGR2HSV);

		//inRange(HSV,Scalar(118,99,72),Scalar(H_MAX,S_MAX,V_MAX),thresholdIm);
		//--inRange(HSV, Scalar(118, 99, 72), Scalar(167, 212, 165), thresholdIm);  // filtrado del morado

		//show frames
		//imshow(windowName2, thresholdIm);
		// imshow(windowName1,HSV);

		//convert frame from BGR to HSV colorspace

		//--blur(thresholdIm, src_gray, Size(3, 3));
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(7, 7), Point(1, 1));
		Mat edges;
		//--Mat dialtemat, erodemat;
		//--erode(src_gray, erodemat, element);
		//--dilate(erodemat, dialtemat, element);


		//blur(dialtemat, src_gray, Size(3, 3));

		threshold(src, imagenThreshold, 120, 255, THRESH_BINARY_INV);
		//erode(imagenThreshold, src_gray, element);

		imshow("gris", imagenThreshold);

		cvtColor(src, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);
		imshow("edges", edges);

		//findContours(edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));




		char k = waitKey(5);
		switch (k)
		{

		case 32: //space hit
			//cvtColor( src, src_gray, CV_BGR2GRAY );
			//blur( src, src, Size(3,3) );
			Mat objeto = imagenThreshold.clone();
			thresh_callback(src);

			sendPoints(resumen2);
			break;

			//case 'x':
			//     stop = true;
			//     break;
		}

		//waitKey(30);
	}
	Serial->Close();



	cout << "GoodBye.." << endl;
	destroyAllWindows();
	waitKey(0);
	return 0;
}
void sendPoints(vector<Point> puntos) {
	char txChar, rxChar;
	if (puntos.size() < 1) {
		cout << "Vector Vacio" << endl;
		return;
	}
	cout << "Enviando Puntos Prueba..." << endl;
	cout << "Puntos Size: " << puntos.size() << endl;
	for (int i = 0; i < puntos.size(); i++) {
		Serial->write((uchar)puntos[i].x);
		Sleep(10);
		Serial->write((uchar)puntos[i].y);
		Sleep(200);
		while (Serial->available()) {
			rxChar = (char)Serial->read();
			cout << rxChar << endl;
		}
	}
}

void thresh_callback(Mat & mat)
{
	//Mat canny_output;	
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Point> lineas;
	Mat erodemat, dilatemat;
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(7, 7), Point(1, 1));

	Canny(mat, canny_output, thresh, thresh * 2, 3);
	//erode(canny_output, erodemat, element);
	//dilate(erodemat, dilatemat, element);

	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
	}

	for (int i = 0; i< contours.size(); i++)
	{
		for (int j = 0; j < contours[i].size(); j++) {
			lineas.push_back(contours[i][j]);
		}
	}
	resume(lineas);
	drawLines(resumen);

	cout << "Resumen 1" << endl;
	cout << resumen << endl;


	//-----vector para disminuir a escala el contorno
	//vector<Point> vectorEscala = escala(resumen);
	//cout << "Vector Escala" << endl;
	//cout << vectorEscala << endl;
	//-------vector disminuido a escala para el robot-----
	//resume2(vectorEscala);
	resumen2=escala(resumen);
	drawLines3(resumen2);
	//drawLines3(vectorEscala);
	cout << "puntos para el robot" << endl;
	cout << resumen2 << endl;
	//cout << resumen2.size() << endl;

}
