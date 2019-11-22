#pragma once
#include "PublicHeader.h"
#include "HelmatDetector.h"

using namespace cv;

class My_Detector
{
public:
	My_Detector();
	My_Detector(string path);
	virtual ~My_Detector();

public:
	Mat GetCannyImg(Mat img);
	Mat GetCannyImg(IplImage* img);
	Mat GetCornerImg(Mat img);
	Mat GetHelmetImg(Mat img);
	Mat GetSmokeImg(Mat img);
	Mat GetSmokeImg_v2(Mat img);

	void smoke_detect();
	Mat smoke_v2(Mat pFrame);
	void video_terminate();

public:
	void SetPath(string path);
private:
	int m_iCornerThresh = 30;
	string curPath;
	HelmatDetector* pd;
	int nFrmNum = 0;

	Mat smoke_process(IplImage* pFrame);
};

