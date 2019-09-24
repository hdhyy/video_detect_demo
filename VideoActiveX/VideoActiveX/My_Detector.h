#pragma once
#include "PublicHeader.h"
#include "PretrainDetector.h"

using namespace cv;

class My_Detector
{
public:
	My_Detector();
	virtual ~My_Detector();

public:
	Mat GetCannyImg(Mat img);
	Mat GetCannyImg(IplImage* img);
	Mat GetCornerImg(Mat img);

	Mat GetHelmetImg(Mat img);

	Mat GetSmokeImg(Mat img);

	void smoke_detect();
	void video_terminate();

public:
	void SetPath(string path);
private:
	int m_iCornerThresh = 30;
	string curPath;
};

