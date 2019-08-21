#include "stdafx.h"
#include "DealWithTensorFlow.h"
#include <opencv2\opencv.hpp>
#include <opencv2\dnn.hpp>
#include <iostream>

DealWithTensorFlow::DealWithTensorFlow()
{
	detectionMat = nullptr;
}


DealWithTensorFlow::DealWithTensorFlow(int height, int width)
{
	inHeight = height;
	inWidth = width;
	detectionMat = nullptr;
	WHRatio = inWidth / (float)inHeight;
}


DealWithTensorFlow::DealWithTensorFlow(CString path)
{
	USES_CONVERSION;
	cv::String str = W2A(path);
	frame = cv::imread(str);
	init();
}

DealWithTensorFlow::DealWithTensorFlow(IplImage* img)
{
	frame = cv::cvarrToMat(img);
	init();
}

void DealWithTensorFlow::init()
{
	frame_size = frame.size();
	inHeight = 300, inWidth = 300;
	WHRatio = inWidth / (float)inHeight;
	weights = "models\\ssd_mobilenet_v2_coco_2018_03_29\\frozen_inference_graph.pb";
	prototxt = "models\\ssd_mobilenet_v2_coco_2018_03_29.pbtxt";
	net = cv::dnn::readNetFromTensorflow(weights, prototxt);
}


DealWithTensorFlow::~DealWithTensorFlow()
{

}

void DealWithTensorFlow::SetCropSize()
{
	if (frame_size.width / (float)frame_size.height > WHRatio)
	{
		cropSize = cv::Size(static_cast<int>(frame_size.height * WHRatio),
			frame_size.height);
	}
	else
	{
		cropSize = cv::Size(frame_size.width,
			static_cast<int>(frame_size.width / WHRatio));
	}
}

void DealWithTensorFlow::Prepare4Train()
{
	cv::Rect crop(cv::Point((frame_size.width - cropSize.width) / 2,
		(frame_size.height - cropSize.height) / 2),
		cropSize);


	cv::Mat blob = cv::dnn::blobFromImage(frame, 1. / 255, cv::Size(inWidth, inHeight));
	//cout << "blob size: " << blob.size << endl;

	net.setInput(blob);
	cv::Mat output = net.forward();
	//cout << "output size: " << output.size << endl;
	detectionMat = new cv::Mat(output.size[2], output.size[3], CV_32F, output.ptr<float>());

	frame = frame(crop);
}

IplImage DealWithTensorFlow::Train()
{
	const char* classNames[100] = { "background","face" };//这个需要根据训练的类别定义
	double confidenceThreshold = 0.20;
	for (int i = 0; i < (*detectionMat).rows; i++)
	{
		float confidence = (*detectionMat).at<float>(i, 2);

		if (confidence > confidenceThreshold)
		{
			size_t objectClass = (size_t)((*detectionMat).at<float>(i, 1));

			int xLeftBottom = static_cast<int>((*detectionMat).at<float>(i, 3) * frame.cols);
			int yLeftBottom = static_cast<int>((*detectionMat).at<float>(i, 4) * frame.rows);
			int xRightTop = static_cast<int>((*detectionMat).at<float>(i, 5) * frame.cols);
			int yRightTop = static_cast<int>((*detectionMat).at<float>(i, 6) * frame.rows);

			std::ostringstream ss;
			ss << confidence;
			cv::String conf(ss.str());

			cv::Rect object((int)xLeftBottom, (int)yLeftBottom,
				(int)(xRightTop - xLeftBottom),
				(int)(yRightTop - yLeftBottom));

			rectangle(frame, object, cv::Scalar(0, 255, 0), 2);
			cv::String label = cv::String(classNames[objectClass]) + ": " + conf;
			int baseLine = 0;
			cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
			rectangle(frame, cv::Rect(cv::Point(xLeftBottom, yLeftBottom - labelSize.height),
				cv::Size(labelSize.width, labelSize.height + baseLine)),
				cv::Scalar(0, 255, 0), CV_FILLED);
			putText(frame, label, cv::Point(xLeftBottom, yLeftBottom),
				cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
		}
	}
	IplImage img = IplImage(frame);
	return img;
}

IplImage result_tf;

IplImage* DealWithTensorFlow::execute()
{
	SetCropSize();
	Prepare4Train();
	result_tf = Train();
	return &result_tf;
}
