#pragma once
#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class PretrainedDet
{
public:
	cv::Mat image;
	string prototxt;
	string weights;
	string img;

	cv::dnn::Net peleeDetection;

public:
	virtual void loadModel();
	void loadImage();
	void showImage();
	virtual void processImage();
	virtual void preprocess();
	virtual void postprocess();
	virtual cv::Mat getProcessedImage(cv::Mat image);
};



