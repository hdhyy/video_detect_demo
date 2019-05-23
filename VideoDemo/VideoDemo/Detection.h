#pragma once
#include "PublicHeader.h"
#include "PretrainDetector.h"
#include <opencv2/opencv.hpp>

using namespace cv;

class Detector
{
public:
	Detector();
	virtual ~Detector();

public:
	Mat GetCannyImg(Mat img);
	Mat GetCannyImg(IplImage *img);
	Mat GetCornerImg(Mat img);

	Mat GetHelmetImg(Mat img);

	void smoke_detect();
private:
	int m_iCornerThresh = 30;
};

