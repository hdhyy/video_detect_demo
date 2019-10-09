#pragma once
#include "PublicHeader.h"
#include "PretrainDetector.h"
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

	void smoke_detect();
	Mat smoke_v2(Mat foreground1);
	void video_terminate();

public:
	Mat P;
	Mat background1;
	void update_smoke_bk(Mat m_background);
private:
	int m_iCornerThresh = 30;
};

