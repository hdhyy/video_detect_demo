#include "pch.h"
#include "PretrainedDet.h"

void PretrainedDet::loadImage(void) {
	this->image = cv::imread(this->img);
}

void  PretrainedDet::loadModel() {
	this->peleeDetection = cv::dnn::readNetFromCaffe(this->prototxt, this->weights);
}

void PretrainedDet::showImage() {
	cv::imshow("results", this->image);
	cv::waitKey();
}

cv::Mat PretrainedDet::getProcessedImage(cv::Mat image) {
	this->image = image;
	this->processImage();
	return this->image;
}

void PretrainedDet::preprocess()
{

}
void PretrainedDet::postprocess()
{

}

void PretrainedDet::processImage()
{

}