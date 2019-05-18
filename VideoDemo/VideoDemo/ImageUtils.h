#pragma once
#include <opencv2\opencv.hpp>

class ImageUtils
{
public:
	ImageUtils(cv::Mat *src_p);
	virtual ~ImageUtils();
	void homo();
	int laplacian_change();
	void mean_shift();
	void eHist();
	void displayCam();
	void highlightMask();
	void showMsrcrVideo();
	cv::Mat* main_msrcr();
protected:
	cv::Mat src;
	cv::Mat dst;
	IplImage* src_m;  //source image
	IplImage* dst_m;  //the dst image after meanshift
	std::vector<double> sigema;
	std::vector<double> weight;
	int m_spatialRad = 10;
	int m_colorRad = 20;
	int m_maxPryLevel = 1;
};

