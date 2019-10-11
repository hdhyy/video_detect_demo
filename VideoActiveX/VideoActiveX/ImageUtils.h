#pragma once

#include <io.h>  
#include <fcntl.h>
#include "PublicHeader.h"
#include "CvxText.hpp"

class ImageUtils
{
public:
	ImageUtils();
	ImageUtils(cv::Mat* src_p);
	virtual ~ImageUtils();
	void homo();
	int laplacian_change();
	void mean_shift();
	void eHist();
	void displayCam();
	void highlightMask();

	void showMsrcrVideo();
	void gray_world(cv::Mat src, cv::Mat& dst);
	cv::Mat main_msrcr();
	cv::Mat main_msr();
	cv::Mat main_msrcr_ex();
	cv::Mat deHaze();
	cv::Mat fast_deHaze();
	cv::Mat deHaze_chai(int r = 81, float eps = 0.001, float w = 0.95, float maxV1 = 0.80);
	cv::Mat brighten();

	cv::Mat video_blur_detect(float thres);
	cv::Mat insect_detect();
	//cv::Mat face_detect();
	void set_src(cv::Mat& src);
	void face_dr();
	void otk_dr();
	void hist_match_test();
	cv::Mat get_hist_match();
protected:
	cv::Mat src;
	cv::Mat dst;
	CvxText* text_ptr;
	//FaceFactory fft;
	//ObjectTracking otk;
	IplImage* src_m = nullptr;  //source image
	IplImage* dst_m = nullptr;  //the dst image after meanshift
	std::vector<double> sigema;
	std::vector<double> weight;
	int m_spatialRad = 10;
	int m_colorRad = 20;
	int m_maxPryLevel = 1;
	int frame_count = 0;
	int text_bling = 0;
	cv::Ptr<cv::SimpleBlobDetector> detector;
	cv::Ptr<cv::BackgroundSubtractorMOG2> bgSubtractor;

	void fastHazeRemoval(const cv::Mat& src, cv::Mat& dst);
	void fastHazeRemoval_1Channel(const cv::Mat& src, cv::Mat& dst);
	void fastHazeRemoval_3Channel(const cv::Mat& src, cv::Mat& dst);
};

