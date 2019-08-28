#pragma once

#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <tracking.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace std;
using namespace cv;

class ObjectTracking
{
public:
	ObjectTracking();
	void play_demo();

protected:
	vector<string> trackerTypes{ "BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT" };
	// create tracker by name
	Ptr<Tracker> createTrackerByName(string trackerType);
	// Fill the vector with random colors
	void getRandomColors(vector<Scalar>& colors, int numColors);
};

