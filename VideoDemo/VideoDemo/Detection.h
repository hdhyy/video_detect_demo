#pragma once
#include "PublicHeader.h"
#include "HelmatDetector.h"
#include "ObjectTracking.h"
#include <opencv2/opencv.hpp>

using namespace cv;

class My_Detector
{
public:
	My_Detector();
	virtual ~My_Detector();

public:
	Mat GetCannyImg(Mat img);
	Mat GetCannyImg(IplImage *img);
	Mat GetCornerImg(Mat img);
	Mat GetHelmetImg(Mat img);
	Mat GetSmokeImg(Mat img);
	Mat GetSmokeImg_v2(Mat img);
	Mat smoke_v2(Mat foreground1);

	Mat GetHumanTrackImg(Mat img);
	void smoke_detect();
	void video_terminate();

public:
	Mat P;
	Mat background1;
	HelmatDetector* pd;
	ObjectTracking* ojt;
	void update_smoke_bk(Mat m_background);
private:
	int m_iCornerThresh = 30;
};

