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

Mat src;
Mat src_gray;
Mat imagenThreshold;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
Mat canny_output;
vector<Point> resumen;
vector<Point> resumen2;
Size tam;

SerialEmul *Serial;

void sendPoints(vector<Point> puntos);  // envio serial de puntos al arduino
/// Function header
void thresh_callback(int, void*);

void drawLines(vector<Point> v) {
	Mat lines = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i< v.size() - 1; i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		line(lines, v[i], v[i + 1], color, 1, 8, 0);
	}

	namedWindow("lineas", CV_WINDOW_AUTOSIZE);
	imshow("lineas", lines);
}

void drawLines2(vector<Point> v) {
	Mat lines = Mat::zeros(canny_output.size(), CV_8UC3);
	cout<<"V size :" <<v.size() << endl;
	for (int i = 0; i< v.size()-1 ; i++)
	{
		Scalar color = Scalar(rng.uniform(5, 255), rng.uniform(5, 255), rng.uniform(5, 255));

		line(lines, v[i], v[i + 1] , color, 1, 8, 0);
	}

	//imshow("Primer resumen", lines);
}

void drawLines3(vector<Point> v) {
	Mat resumida = Mat::zeros(30, 30, CV_8UC3);
	for (int i = 0; i< v.size() - 1; i++)
	{
		Scalar color = Scalar(rng.uniform(5, 255), rng.uniform(5, 255), rng.uniform(5, 255));

		line(resumida, v[i], v[i + 1], color, 1, 8, 0);
	}

	imshow("Segundo resumen en escala", resumida);
}


void resume(vector<Point> puntos) {
	int comp1 = puntos[0].x;
	int comp2 = puntos[0].y;
	int j = 0;
	resumen.clear();
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
	resumen2.clear();
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
		escalado.push_back(Point(resumido[i].x / 9, resumido[i].y / 9));
	}

	return escalado;
}




/** @function main */
int main(int argc, char** argv)
{
	/// Load source image and convert it to gray
	src = imread("C:\\Users\\eddy_\\Pictures\\circle.jpg", CV_LOAD_IMAGE_COLOR);
	
	if (src.data == NULL) {
		cout << "Error: Image 	could not be loaded, Invalid Path" << endl; 		getchar();
		return-1;
	};

	/*
	//Conecting to serial device
	Serial = new SerialEmul();
	int port = -1;
	char c_baud;
	int baud;
	
	while(port<1 || port>99){
	cout<<"*Enter Port # : ";
	cin>>port;
	}
	cout<<endl;
	cout<<"*Select Baud Rate :  \n-(default 9600)"<<endl;
	cout<<"a) 4800\nb) 9600\nc) 14400\nd) 19200\ne) 28800\nf) 38400\ng) 57600\nh) 115200"<<endl;
	cin>>c_baud;

	switch(c_baud){
	case 'a': baud = 4800 ; break;
	case 'b': baud = 9600; break;
	case 'c': baud = 14400; break;
	case 'd': baud = 19200; break;
	case 'e': baud = 28800; break;
	case 'f': baud = 38400; break;
	case 'g': baud = 57600; break;
	case 'h': baud = 115200; break;
	default : baud = 9600; break;
	}
	cout<<"-------------------------------"<<endl;
	if(!Serial->Open(port,baud)){
		cout<<"Error: Could not connect to COM"<<port<<endl;
		exit(-1l);
	}
	*/

	tam = src.size();
	/// Convert image to gray and blur it
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3));
	threshold(src_gray, imagenThreshold, 200, 255, THRESH_BINARY_INV);

	imshow("source", src);

	thresh_callback(0, 0);
	vector<Point> puntosPrueba = { { 9,0 },{ 4,3 },{ 1,8 },{ 1,13 },{ 4,18 },{ 12,20 },{ 17,17 },{ 20,12 },{ 20,7 },{ 17,2 },{ 9,0 } };
	
	//cout << (UCHAR)puntosPrueba[0].x << endl;
	//cout << (UCHAR)puntosPrueba[0].y << endl;
	//sendPoints(puntosPrueba);
	vector<Point> puntosPrueba2 = { {1,1}, {2,2},{3,3}, {4,4},{5,5}, {6,6},{7,7}, {8,8},{9,9}, {10,10},{ 9,9 },{ 8,8 } ,{ 7,7 },{ 6,6 },{ 5,5 },{ 4,4},{3,3},{ 2,2},{1,1} };
	//sendPoints(puntosPrueba2);
	


	/*
	uchar a = 0x02;
	Serial->write(0x0);
	Serial->write(0x0);
	Sleep(5000);
	Serial->write(0x4);
	Serial->write(0xA);
	Sleep(5000);
	Serial->write(0xA);
	Serial->write(0x8);
	Sleep(5000);
	Serial->write(0x5);
	Serial->write(0x2);
	Sleep(10);
	while (Serial->available()) {
		cout << (int)Serial->read() << endl;

	}*/

	char key=0;
	VideoCapture camera = VideoCapture(0);
	namedWindow("WebCam");
	Mat currentImage = Mat(170, 170, CV_8UC3);
	Mat webcam = Mat(170, 170, CV_8UC3);
	imshow("WebCam", webcam);
	
	while (key != 27) {
		camera.read(webcam);
		imshow("WebCam", webcam);

		key = waitKey(5);
		switch (key) {
		case 32: 
			src = webcam.clone();
			cvtColor(src, src_gray, CV_BGR2GRAY);
			blur(src_gray, src_gray, Size(3, 3));
			threshold(src_gray, imagenThreshold, 200, 255, THRESH_BINARY_INV);
			thresh_callback(0, 0);
			imshow("source", src);
			break;
		default:break;
		}
		Sleep(15);
	}
	
	


	//Serial->Close();
	
	

	cout << "GoodBye.." << endl;
	destroyAllWindows();
	waitKey(0);
	return(0);
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
			rxChar = (uchar)Serial->read();
			cout << rxChar<<endl;
		}
	}
}


/** @function thresh_callback */
void thresh_callback(int, void*)
{
	//Mat canny_output;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Point> lineas;

	Canny(src_gray, canny_output, thresh, thresh * 2, 3);

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

	/// Show in a window
 	imshow("Contours", drawing);


	//-------lineas del contorno
	//drawLines(lineas);
	resume(lineas);
	//cout << "Lineas" << endl;
	//cout << lineas << endl;
	//cout << lineas.size() << endl;
	//cout << endl;

	//------vector de lineas resumido a un solo contorno
	drawLines2(resumen);
	//cout << "primer resumen" << endl;
	//cout << resumen << endl;
	//cout << resumen.size() << endl;

	//------Solo para sacar el valor de la escala------
	//cout << tam.height / 20 << endl;
	//cout << tam.width / 20 << endl;


	//-----vector para disminuir a escala el contorno
	vector<Point> vectorEscala = escala(resumen);


	//-------vector disminuido a escala para el robot-----
	//resumen2.clear();
	//resume2(vectorEscala);
	//drawLines3(resumen2);
	//cout << "puntos para el robot" << endl;
	//cout << resumen2 << endl;
	//cout << resumen2.size() << endl;



	//Sleep(50);
}