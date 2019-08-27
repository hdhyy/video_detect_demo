#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <seeta/FaceEngine.h>
#include <seeta/Struct_cv.h>
#include <seeta/Struct.h>

#define SEETAFACE2 10001

class FaceFactory
{
public:

	FaceFactory();
	seeta::ModelSetting::Device device = seeta::ModelSetting::GPU;
	int id = 0;
	seeta::ModelSetting *FD_model; 
	seeta::ModelSetting *PD_model; 
	seeta::ModelSetting *FR_model; 
	seeta::FaceEngine *engine;

	// recognization threshold
	float threshold = 0.6f;
	std::vector<std::string> GalleryImageFilename = { "1.jpg" };
	std::vector<int64_t> *GalleryIndex;

public:
	void play_demo();
	cv::Mat get_detect_result(int detect_type, cv::Mat* src);
protected:
	std::map<int64_t, std::string> GalleryIndexMap;
};

