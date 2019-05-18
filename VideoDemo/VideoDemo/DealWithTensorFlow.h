#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

class DealWithTensorFlow
{
public:
	DealWithTensorFlow();
	DealWithTensorFlow(size_t height, size_t width);
	DealWithTensorFlow(CString path);
	DealWithTensorFlow(IplImage *img);
	IplImage* execute();
	virtual ~DealWithTensorFlow();

protected:
	void init();
	size_t inWidth;
	size_t inHeight;
	float WHRatio;
	cv::Mat frame;
	cv::Size frame_size;
	cv::String weights;
	cv::String prototxt;
	cv::dnn::Net net;
	cv::Size cropSize;
	cv::Mat *detectionMat;
	void SetCropSize();
	void Prepare4Train();
	IplImage* Train();

};

