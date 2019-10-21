#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "PretrainedDet.h"
using namespace std;

#define CONFTHRESHOLD 0.1
class HelmatDetector : public PretrainedDet
{
public:
	string labelNames[5];
	vector<string> classes;
	vector<cv::Scalar> showColor;

	float inputscale;
	cv::Scalar mean;
	cv::Size caffeInputSize;
	cv::Mat preprocessed;

	vector<cv::Mat> detections;
	cv::Mat inputBlob;

	HelmatDetector(string labelNames[5], string prototxt, string weights);
	HelmatDetector(string labelNames[5], string prototxt, string weights, string img);
	void processImage();

	void preprocess();
	void postprocess();

private:
	vector<cv::String> getOutpusNames();
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);
	void resizeImage(cv::Mat& frame, cv::Mat& dst, cv::Size dims);
};
