#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <tracking.hpp>
#include <string>
#include <iostream>

#include "PretrainedDet.h"

using namespace std;
using namespace cv;

class ObjectTracking : public PretrainedDet
{
public:
	// Initialize the parameters
	float confThreshold = 0.5; // Confidence threshold
	float nmsThreshold = 0.4;  // Non-maximum suppression threshold
	int inpWidth = 416;  // Width of network's input image
	int inpHeight = 416; // Height of network's input image
	vector<string> classes;

	dnn::Net net;

public:
	ObjectTracking();
	ObjectTracking(string prototxt, string weights);
	void play_demo();

protected:
	vector<string> trackerTypes{ "BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT" };
	// create tracker by name
	Ptr<Tracker> createTrackerByName(string trackerType);
	// Fill the vector with random colors
	void getRandomColors(vector<Scalar>& colors, int numColors);
	void postprocess();
	// Remove the bounding boxes with low confidence using non-maxima suppression
	void postprocess(Mat& frame, const vector<Mat>& outs);
	// Draw the predicted bounding box
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);
	// Get the names of the output layers
	vector<String> getOutputsNames(const dnn::Net& net);

	void processImage();

	void loadModel();
};

