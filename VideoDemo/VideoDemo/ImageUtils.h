#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <array>
#include <map>
#include <iostream>

#include <io.h>  
#include <fcntl.h>

#include "FaceFactory.h"

class ImageUtils
{
public:
	ImageUtils();
	ImageUtils(cv::Mat *src_p);
	virtual ~ImageUtils();
	void homo();
	int laplacian_change();
	void mean_shift();
	void eHist();
	void displayCam();
	void highlightMask();
	void showMsrcrVideo();
	void gray_world(cv::Mat src, cv::Mat &dst);
	cv::Mat main_msrcr();
	cv::Mat main_msr();
	cv::Mat main_msrcr_ex();
	cv::Mat deHaze();
	cv::Mat ImageUtils::video_blur_detect();
	cv::Mat insect_detect();
	cv::Mat face_detect();
	void set_src(cv::Mat& src);
	void face_dr();
protected:
	cv::Mat src;
	cv::Mat dst;
	FaceFactory fft;
	IplImage* src_m;  //source image
	IplImage* dst_m;  //the dst image after meanshift
	std::vector<double> sigema;
	std::vector<double> weight;
	int m_spatialRad = 10;
	int m_colorRad = 20;
	int m_maxPryLevel = 1;
	int frame_count;
	cv::Ptr<cv::SimpleBlobDetector> detector;
	cv::Ptr<cv::BackgroundSubtractorMOG2> bgSubtractor;
};

