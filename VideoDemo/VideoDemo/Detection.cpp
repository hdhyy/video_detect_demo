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
	double mean;
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

int nFrmNum = 0;
IplImage* pFrImg = NULL;
IplImage* pBkImg = NULL;
IplImage* smokeimg = NULL;
IplImage* dst = NULL;
IplImage* dstt = NULL;
int i, j;
int step, step_rgb, channels, cd, cdrgb, b, g, r;
double minv, I, maxv;

CvMemStorage *storage = cvCreateMemStorage(0);
CvSeq *contours = 0;

CvMat* pFrrMat = NULL;
CvMat* dsttMat = NULL;
CvMat* pFrameMat = NULL;
CvMat* pFrMat = NULL;
CvMat* pBkMat = NULL;

void Detector::video_terminate()
{
	nFrmNum = 0;
	cvReleaseImage(&pFrImg);
	cvReleaseImage(&pBkImg);
	cvReleaseImage(&dstt);
	cvReleaseImage(&smokeimg);
	cvReleaseMat(&pFrameMat);
	cvReleaseMat(&pFrMat);
	cvReleaseMat(&pBkMat);
	cvReleaseMat(&dsttMat);
}
Mat smoke_process(IplImage *pFrame)
{
	nFrmNum++;
	const auto T1 = 40;  //15-55   
	const auto T2 = 210;  //190-245   
	const auto T3 = 245;  //200-255   
	const auto T4 = 45;
	const auto threshold = 10;
	uchar *data, *data1;
	dst = cvCreateImage(CvSize(pFrame->width, pFrame->height), pFrame->depth, pFrame->nChannels);
	dstt = cvCreateImage(CvSize(pFrame->width, pFrame->height), pFrame->depth, 1);
	smokeimg = cvCreateImage(CvSize(pFrame->width, pFrame->height), pFrame->depth, 1);
	step = pFrame->widthStep;
	channels = pFrame->nChannels;
	data = (uchar *)pFrame->imageData;
	data1 = (uchar *)dstt->imageData;
	step_rgb = dstt->widthStep;
	for (i = 0; i < pFrame->height; i++)
		for (j = 0; j < pFrame->width; j++)
		{
			cd = i * step + j * channels;
			cdrgb = i * step_rgb + j;
			b = data[cd], g = data[cd + 1], r = data[cd + 2];
			minv = __min(__min(r, g), b);
			maxv = __max(__max(r, g), b);
			I = (r + g + b) / 3;

			if (((T2 < I) && (T3 > I) && (maxv - minv) < T4) || ((T2 < I) && (T3 > I) && (maxv = r)))
				data1[cdrgb] = 255;
			else
				data1[cdrgb] = 0;
		}
	cvCopy(pFrame, dst, dstt);

	if (nFrmNum == 1)
	{
		pBkImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1);
		pFrImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1);
		pBkMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
		pFrMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
		pFrameMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
		pFrrMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
		dsttMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);

		cvCvtColor(pFrame, pBkImg, CV_BGR2GRAY);
		cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);

		cvConvert(pFrImg, pFrameMat);
		cvConvert(pFrImg, pFrMat);
		cvConvert(pFrImg, pBkMat);
	}
	else
	{
		cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);
		cvConvert(pFrImg, pFrameMat);
		cvConvert(dstt, dsttMat);

		cvSmooth(pFrameMat, pFrameMat, CV_GAUSSIAN, 3, 0, 0);

		cvAbsDiff(pFrameMat, pBkMat, pFrMat);

		cvThreshold(pFrMat, pFrImg, threshold, 255.0, CV_THRESH_BINARY);

		cvAbsDiff(dsttMat, pFrMat, pFrrMat);
		cvAbsDiff(pFrrMat, dsttMat, pFrMat);
		cvThreshold(pFrMat, pFrImg, threshold, 255.0, CV_THRESH_BINARY);

		cvErode(pFrImg, pFrImg, 0, 1);
		cvDilate(pFrImg, pFrImg, 0, 1);

		cvConvert(pBkMat, pBkImg);

		cvCopy(pFrImg, smokeimg, smokeimg);

		double tmparea = 0.0;
		CvSeq* max_contours = nullptr;

		cvFindContours(pFrImg, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		for (; contours != 0; contours = contours->h_next)
		{
			if (fabs(cvContourArea(contours)) > tmparea)
			{
				tmparea = fabs(cvContourArea(contours));
				max_contours = contours;
			}
		}
		CvRect Rect = cvBoundingRect(max_contours, 0);

		if ((Rect.width / Rect.height) < 2)
			cvRectangle(pFrame, cvPoint(Rect.x, Rect.y), cvPoint(Rect.x + Rect.width, Rect.y + Rect.height), CV_RGB(255, 0, 0), 1, 20, 0);
	}

	return cvarrToMat(pFrame);
}

Mat Detector::GetSmokeImg(Mat img)
{
	IplImage *smoke_img = &(IplImage)img;
	return smoke_process(smoke_img);
}

void Detector::smoke_detect()
{
	const auto T1 = 40;  //15-55   
	const auto T2 = 210;  //190-245   
	const auto T3 = 245;  //200-255   
	const auto T4 = 45;
	const auto threshold = 10;

	IplImage* pFrame = NULL;
	IplImage* pFrImg = NULL;
	IplImage* pBkImg = NULL;
	IplImage* smokeimg = NULL;
	IplImage* dst = NULL;
	IplImage* dstt = NULL;
	int i, j;
	int step, step_rgb, channels, cd, cdrgb, b, g, r;
	double minv, I, maxv;
	uchar *data, *data1;

	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = 0;

	CvMat* pFrrMat = NULL;
	CvMat* dsttMat = NULL;
	CvMat* pFrameMat = NULL;
	CvMat* pFrMat = NULL;
	CvMat* pBkMat = NULL;

	int nFrmNum = 0;

	cvNamedWindow("video", 1);
	cvNamedWindow("background", 1);
	cvNamedWindow("smoke", 1);
	cvNamedWindow("smoke_detection", 1);
	cvNamedWindow("RGB", 1);
	cvNamedWindow("foreground", 1);

	cvMoveWindow("video", 0, 0);
	cvMoveWindow("background", 200, 0);
	cvMoveWindow("smoke", 400, 0);
	cvMoveWindow("RGB", 600, 0);
	cvMoveWindow("foreground", 800, 0);
	cvMoveWindow("smoke_detection", 1000, 0);

	VideoCapture *vcap = new VideoCapture("video/59.avi");
	if (!vcap->isOpened())
	{
		fprintf(stderr, "Can not open video file %s\n", "video/smoke.avi");
		return;
	}
	double fps = vcap->get(CV_CAP_PROP_FPS);
	Mat pMat;
	(*vcap) >> pMat;
	pFrame = &(IplImage)pMat;
	while (!pMat.empty())
	{
		dst = cvCreateImage(CvSize(pFrame->width, pFrame->height), pFrame->depth, pFrame->nChannels);
		dstt = cvCreateImage(CvSize(pFrame->width, pFrame->height), pFrame->depth, 1);
		smokeimg = cvCreateImage(CvSize(pFrame->width, pFrame->height), pFrame->depth, 1);
		step = pFrame->widthStep;
		channels = pFrame->nChannels;
		data = (uchar *)pFrame->imageData;
		data1 = (uchar *)dstt->imageData;
		step_rgb = dstt->widthStep;
		for (i = 0; i < pFrame->height; i++)
			for (j = 0; j < pFrame->width; j++)
			{
				cd = i * step + j * channels;
				cdrgb = i * step_rgb + j;
				b = data[cd], g = data[cd + 1], r = data[cd + 2];
				minv = __min(__min(r, g), b);
				maxv = __max(__max(r, g), b);
				I = (r + g + b) / 3;

				if (((T2 < I) && (T3 > I) && (maxv - minv) < T4) || ((T2 < I) && (T3 > I) && (maxv = r)))
					data1[cdrgb] = 255;
				else
					data1[cdrgb] = 0;
			}
		cvCopy(pFrame, dst, dstt);
		nFrmNum++;

		if (nFrmNum == 1)
		{
			pBkImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1);
			pFrImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1);
			pBkMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
			pFrMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
			pFrameMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
			pFrrMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
			dsttMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);

			cvCvtColor(pFrame, pBkImg, CV_BGR2GRAY);
			cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);

			cvConvert(pFrImg, pFrameMat);
			cvConvert(pFrImg, pFrMat);
			cvConvert(pFrImg, pBkMat);
		}
		else
		{
			cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);
			cvConvert(pFrImg, pFrameMat);
			cvConvert(dstt, dsttMat);

			cvSmooth(pFrameMat, pFrameMat, CV_GAUSSIAN, 3, 0, 0);

			cvAbsDiff(pFrameMat, pBkMat, pFrMat);

			cvThreshold(pFrMat, pFrImg, threshold, 255.0, CV_THRESH_BINARY);

			cvShowImage("RGB", dstt);
			cvShowImage("foreground", pFrImg);

			cvAbsDiff(dsttMat, pFrMat, pFrrMat);
			cvAbsDiff(pFrrMat, dsttMat, pFrMat);
			cvThreshold(pFrMat, pFrImg, threshold, 255.0, CV_THRESH_BINARY);

			cvErode(pFrImg, pFrImg, 0, 1);
			cvDilate(pFrImg, pFrImg, 0, 1);

			cvConvert(pBkMat, pBkImg);

			if (nFrmNum % 30 == 0)
			{
				cvCopy(pFrImg, smokeimg, smokeimg);
				cvShowImage("smoke", smokeimg);
			}
			double tmparea = 0.0;
			CvSeq* max_contours = nullptr;

			cvFindContours(pFrImg, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
			for (; contours != 0; contours = contours->h_next)
			{
				if (fabs(cvContourArea(contours)) > tmparea)
				{
					tmparea = fabs(cvContourArea(contours));
					max_contours = contours;
				}
			}
			CvRect Rect = cvBoundingRect(max_contours, 0);

			if ((Rect.width / Rect.height) < 2)
				cvRectangle(pFrame, cvPoint(Rect.x, Rect.y), cvPoint(Rect.x + Rect.width, Rect.y + Rect.height), CV_RGB(255, 0, 0), 1, 20, 0);

			cvShowImage("video", pFrame); //video   
			cvShowImage("background", pBkImg);
			cvShowImage("smoke_detection", pFrImg);

			if (cvWaitKey(2) >= 0)
				break;
		}
		Sleep((DWORD)(1000 / fps));//间隔33毫秒
		(*vcap) >> pMat;
		pFrame = &(IplImage)pMat;
	}
	cvWaitKey(0);

	cvDestroyWindow("video");
	cvDestroyWindow("background");
	cvDestroyWindow("RGB");
	cvDestroyWindow("smoke");
	cvDestroyWindow("smoke_detection");
	cvDestroyWindow("foreground");

	cvReleaseImage(&pFrImg);
	cvReleaseImage(&pBkImg);
	cvReleaseImage(&dstt);
	cvReleaseImage(&smokeimg);
	cvReleaseMat(&pFrameMat);
	cvReleaseMat(&pFrMat);
	cvReleaseMat(&pBkMat);
	cvReleaseMat(&dsttMat);
	vcap->release();
}
