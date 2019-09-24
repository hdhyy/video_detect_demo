#pragma once
#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#define CONFTHRESHOLD 0.1
class PretrainDetector
{
public:
	string labelNames[5];
	vector<string> classes;
	vector<cv::Scalar> showColor;
	cv::dnn::Net peleeDetection;
	float inputscale;
	cv::Scalar mean;
	cv::Size caffeInputSize;
	cv::Mat preprocessed;
	cv::Mat image;
	string prototxt;
	string weights;
	string img;
	vector<cv::Mat> detections;
	cv::Mat inputBlob;

	PretrainDetector(string labelNames[5], string prototxt, string weights, cv::Mat image);
	PretrainDetector(string labelNames[5], string prototxt, string weights, string img);
	cv::Mat getProcessedImage();
	void processImage();
	void showImage();
	void loadModel();
	void loadImage();
	void preprocess();
	void postprocess();

private:
	vector<cv::String> getOutpusNames();
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);
	void resizeImage(cv::Mat& frame, cv::Mat& dst, cv::Size dims);
};


