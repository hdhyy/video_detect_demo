#include "stdafx.h"
#include "HelmatDetector.h"


void HelmatDetector::processImage() {
	preprocess();
	this->inputBlob = cv::dnn::blobFromImage(this->preprocessed, 1.0, this->caffeInputSize, cv::Scalar::all(0), false, false);
	this->peleeDetection.setInput(this->inputBlob);
	this->peleeDetection.forward(this->detections, getOutpusNames());
	postprocess();
}

HelmatDetector::HelmatDetector(string labelNames[5], string prototxt, string weights, string img) {
	for (int i = 0; i < 5; i++) {
		this->labelNames[i] = labelNames[i];
	}
	this->classes = vector<string>(this->labelNames, this->labelNames + 5);
	this->showColor = { cv::Scalar(0,255,255),cv::Scalar(0,0,255),
									cv::Scalar(255,0,0),cv::Scalar::all(255),
									cv::Scalar(0,255,0) };
	this->mean = cv::Scalar(103.94, 116.78, 123.68);
	this->caffeInputSize = cv::Size(304, 304);
	this->inputscale = 0.017f;
	this->prototxt = prototxt;
	this->weights = weights;
	this->img = img;
	loadModel();
	loadImage();
}

HelmatDetector::HelmatDetector(string labelNames[5], string prototxt, string weights) {
	for (int i = 0; i < 5; i++) {
		this->labelNames[i] = labelNames[i];
	}
	this->classes = vector<string>(this->labelNames, this->labelNames + 5);
	this->showColor = { cv::Scalar(0,255,255),cv::Scalar(0,0,255),
									cv::Scalar(255,0,0),cv::Scalar::all(255),
									cv::Scalar(0,255,0) };
	this->mean = cv::Scalar(103.94, 116.78, 123.68);
	this->caffeInputSize = cv::Size(304, 304);
	this->inputscale = 0.017f;
	this->prototxt = prototxt;
	this->weights = weights;
	//this->image = image;
	loadModel();
	//loadImage();
}

vector<cv::String> HelmatDetector::getOutpusNames()
{
	static vector<cv::String> names;
	if (names.empty())
	{
		vector<int> outLayers = this->peleeDetection.getUnconnectedOutLayers();
		vector<cv::String> layersNames = this->peleeDetection.getLayerNames();
		names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			names[i] = layersNames[outLayers[i] - 1];
	}
	return names;
}

void HelmatDetector::preprocess()
{
	this->image.convertTo(this->preprocessed, CV_32F, 1.0 / 255.0, 0.0);
	resizeImage(this->preprocessed, this->preprocessed, this->caffeInputSize);
	this->preprocessed.convertTo(this->preprocessed, CV_32F, 255.0, 0);
	cv::subtract(this->preprocessed, this->mean, this->preprocessed);
	this->preprocessed = this->preprocessed * this->inputscale;
}

void HelmatDetector::postprocess()
{
	static vector<int> outLayers = this->peleeDetection.getUnconnectedOutLayers();
	static string outLayerType = this->peleeDetection.getLayer(outLayers[0])->type;

	vector<int> classIds;
	vector<float> confidences;
	vector<cv::Rect> boxes;

	CV_Assert(this->detections.size() == 1);
	float* data = (float*)this->detections[0].data;
	for (size_t i = 0; i < this->detections[0].total(); i += 7)
	{
		float confidence = data[i + 2];
		if (confidence > CONFTHRESHOLD)
		{
			int left = (int)(data[i + 3] * this->image.cols);
			int top = (int)(data[i + 4] * this->image.rows);
			int right = (int)(data[i + 5] * this->image.cols);
			int bottom = (int)(data[i + 6] * this->image.rows);
			int width = right - left + 1;
			int height = bottom - top + 1;
			int classId = (int)(data[i + 1]) - 1;

			drawPred(classId, confidence, left, top, right, bottom, this->image);
			classIds.push_back(classId);
			boxes.push_back(cv::Rect(left, top, width, height));
			confidences.push_back(confidence);
		}
	}

}

void HelmatDetector::drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame)
{

	cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), this->showColor[classId]);
	string label = this->classes[classId] + ":" + cv::format("%.2f", conf);
	int baseLine;
	cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	top = max(top, labelSize.height);
	cv::rectangle(frame, cv::Point(left, top - labelSize.height),
		cv::Point(left + labelSize.width, top + baseLine), this->showColor[classId], cv::FILLED);
	cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar());
}

void HelmatDetector::resizeImage(cv::Mat& frame, cv::Mat& dst, cv::Size dims)
{
	double max, min;
	int idx_min[2] = { 255, 255 }, idx_max[2] = { 255, 255 };
	cv::Mat tempImage;
	frame.copyTo(tempImage);
	tempImage = tempImage.reshape(1);
	cv::minMaxIdx(tempImage, &min, &max, idx_min, idx_max);

	cv::Mat imageStd;
	cv::Mat resizedStd;
	cv::subtract(frame, cv::Scalar::all(min), imageStd);

	imageStd = imageStd / (max - min);
	cv::resize(imageStd, resizedStd, dims);
	resizedStd = resizedStd * (max - min);
	cv::add(resizedStd, cv::Scalar::all(min), dst);

}

int main(int argc, char *argv[]) {
	if (argc != 4)
		cout << "usage example: main.exe prototext.prototxt weights.caffemodel image.jpg" << endl;
	else {
		string labelNames[5] = { "Y", "Y", "Y", "Y", "N" };
		string prototxt = argv[1];
		string weights = argv[2];
		string img = argv[3];
		HelmatDetector d = HelmatDetector(labelNames, prototxt, weights, img);
		cv::Mat postImage = d.getProcessedImage(d.image);
		d.showImage();
	}
	return 0;
}