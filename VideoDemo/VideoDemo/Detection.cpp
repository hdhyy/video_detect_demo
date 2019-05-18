#include "stdafx.h"
#include "Detection.h"


Detector::Detector()
{
}


Detector::~Detector()
{
}

Mat Detector::GetCannyImg(Mat img) {
	Mat dst_img, edge, gray_img;

	dst_img.create(img.size(), img.type());
	if (img.channels() == 3) {
		cvtColor(img, gray_img, COLOR_BGR2GRAY);
	}
	blur(gray_img, edge, Size(3, 3));
	Canny(edge, edge, 3, 9, 3);
	cvtColor(edge, edge, COLOR_GRAY2BGR);
	return edge;
}

Mat Detector::GetCannyImg(IplImage *img)
{
	//Canny边缘检测算子
	IplImage *gray = 0, *edge = 0;
	gray = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 1);
	edge = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 1);
	cvCvtColor(img, gray, CV_BGR2GRAY);
	cvCanny(gray, edge, 30, 100, 3);
	cvCvtColor(edge, img, CV_GRAY2BGR);
	cvReleaseImage(&gray);
	cvReleaseImage(&edge);
	return cvarrToMat(img);
}

Mat Detector::GetCornerImg(Mat img) {
	Mat dst_img, norm_img, scaled_img, img1, gray_img;
	img1 = img.clone();
	dst_img = Mat::zeros(img.size(), CV_32FC1);

	if (img.channels() == 3) {
		cvtColor(img, gray_img, COLOR_BGR2GRAY);
	}
	cornerHarris(gray_img, dst_img, 2, 3, 0.04, BORDER_DEFAULT);

	normalize(dst_img, norm_img, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(norm_img, scaled_img);

	// 将检测到的，且符合阈值条件的角点绘制出来  
	Mat gray, mat_mean, mat_stddev;
	meanStdDev(norm_img, mat_mean, mat_stddev);
	double mean, std;
	mean = mat_mean.at<double>(0, 0);
	for (int j = 0; j < norm_img.rows; j++)
	{
		for (int i = 0; i < norm_img.cols; i++)
		{
			if ((int)norm_img.at<float>(j, i) > m_iCornerThresh + mean)
			{
				circle(img1, Point(i, j), 5, Scalar(10, 10, 255), 2, 8, 0);
			}
		}
	}
	return img1;
}

Mat Detector::GetHelmetImg(Mat img)
{
	string labelNames[5] = { "Y", "Y", "Y", "Y", "N" };
	string prototxt = "models/pelee/deploy_inference.prototxt";
	string weight = "models/pelee/pelee_SSD_304x304_map78.caffemodel";
	PretrainDetector pd(labelNames, prototxt, weight, img);
	Mat result = pd.getProcessedImage();
	return result;
}
