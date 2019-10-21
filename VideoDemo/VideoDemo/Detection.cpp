#include "stdafx.h"
#include "Detection.h"


My_Detector::My_Detector()
{
	P = Mat(256, 80, CV_32FC1);
	ifstream ifile;
	ifile.open("data.txt");
	for (int j = 0; j < 256; j++)
	{
		for (int i = 0; i < 79; i++)
		{
			string value;
			getline(ifile, value, '\t');
			float type = atof(value.c_str());
			//P1[j][i] = type;
			P.at<float>(j, i) = type;
		}
		int i = 79;
		string value1;
		getline(ifile, value1, '\n');
		float type = atof(value1.c_str());
		//P1[j][i] = type;
		P.at<float>(j, i) = type;
	}
	ifile.close();

	string labelNames[5] = { "Y", "Y", "Y", "Y", "N" };
	string prototxt = "models/pelee/deploy_inference.prototxt";
	string weight = "models/pelee/pelee_SSD_304x304_map78.caffemodel";
	pd = new HelmatDetector(labelNames, prototxt, weight);

	// Give the configuration and weight files for the model
	String modelConfiguration = "models/yolo3/yolov3.cfg";
	String modelWeights = "models/yolo3/yolov3.weights";
	ojt = new ObjectTracking(modelConfiguration, modelWeights);
}


My_Detector::~My_Detector()
{
	delete pd;
	delete ojt;
}

Mat My_Detector::GetCannyImg(Mat img) {
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

Mat My_Detector::GetCannyImg(IplImage *img)
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

Mat My_Detector::GetCornerImg(Mat img) {
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

Mat My_Detector::GetHelmetImg(Mat img)
{
	Mat result = pd->getProcessedImage(img);
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

void My_Detector::video_terminate()
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

//对矩阵求和
double sumMat(Mat& inputImg)
{
	double sum = 0.0;
	int rowNumber = inputImg.rows;
	int colNumber = inputImg.cols * inputImg.channels();
	for (int i = 0; i < rowNumber; i++)
	{
		uchar* data = inputImg.ptr<uchar>(i);
		for (int j = 0; j < colNumber; j++)
		{
			sum = data[j] + sum;
		}
	}
	return sum;
}
//对向量求和
float sumvec(Mat& inputImg)
{
	float sum = 0.0;
	for (int i = 0; i < 256; i++)
	{
		sum = *(float*)(inputImg.ptr<float>(i) + 0) + sum;
	}
	return sum;
}

/*************************************************
//  Method:    convertTo3Channels
//  Description: 将单通道图像转为三通道图像
//  Returns:   cv::Mat
//  Parameter: binImg 单通道图像对象
*************************************************/
Mat convertTo3Channels(const Mat& binImg)
{
	Mat three_channel = Mat::zeros(binImg.rows, binImg.cols, CV_8UC3);
	vector<Mat> channels;
	for (int i = 0; i < 3; i++)
	{
		channels.push_back(binImg);
	}
	merge(channels, three_channel);
	return three_channel;
}

void My_Detector :: update_smoke_bk(Mat background_m)
{
	background1 = background_m;
}

Mat My_Detector::smoke_v2(Mat foreground1)
{
	//读取图片，分别是背景图片和前景图片，并转换成CV_32FC1
	vector <Mat> channels;
	vector <Mat> channel1s;
	//Mat background1 = imread("smoke_bk.jpg");
	//Mat foreground1 = imread("");//原始图片
	Mat background, foreground;
	cvtColor(background1, background, CV_BGR2GRAY);
	cvtColor(foreground1, foreground, CV_BGR2GRAY);//转成灰度图
	//split(background1, channels);
	//// background = channels[0];
	//channels[2].copyTo(background);
	//split(foreground1, channel1s);
	//// foreground = channel1s[0];
	//channel1s[2].copyTo(foreground);

	//  cv::Mat background(480, 640, CV_8UC1);
	// 	cv::Mat foreground(480, 640, CV_8UC1);
	// 	for(int i=0;i<background1.rows;i++)
	//     {
	//         for(int j=0;j<background1.cols;j++)
	//         {
	//             background.at(i, j)=(background1.at(i,j))[2];
	//             background.at(i, j)=(foreground1.at(i,j))[2];
	//         }
	//     }
	background.convertTo(background, CV_32FC1);
	foreground.convertTo(foreground, CV_32FC1);
	
	const int width = 320;
	const int height = 240;
	cv::Mat b(height, width, CV_32FC1);
	//background.convertTo(b, CV_32FC1);
	resize(background, b, b.size(), 0, 0, INTER_LINEAR);
	cv::Mat f(height, width, CV_32FC1);
	//foreground.convertTo(f, CV_32FC1);//转成CV_32FC1
	resize(foreground, f, f.size(), 0, 0, INTER_LINEAR);
	int block = 16;
	int m = b.rows;//m是480是行，n是640是列
	int n = b.cols;
	int blockSize = block * block;
	vector<float> alpha, s;

	cv::Mat f3(256, 1, CV_32FC1);
	cv::Mat b3(256, 1, CV_32FC1);
	cv::Mat f31(256, 1, CV_32FC1);
	cv::Mat b31(256, 1, CV_32FC1);
	cv::Mat ones(256, 1, CV_32FC1);
	cv::Mat zarb(256, 1, CV_32FC1);
	cv::Mat f3f3(256, 1, CV_32FC1);
	cv::Mat b3b3(256, 1, CV_32FC1);

	for (int i = 0; i < n; i = i + block)//i是列是x，j是行是y   之前是n
	{
		for (int j = 0; j < m; j = j + block)//i是列是x，j是行是y   之前是m
		{
			Mat src = f(Range(j, j + block), Range(i, i + block));//前面是行，后面是列
			Mat drc = b(Range(j, j + block), Range(i, i + block));//j~j+block-1

			Mat f1, b1;
			src.copyTo(f1);
			drc.copyTo(b1);//
			//std::cout << "f1.type() = " << f1.type() << std::endl;
			f31 = f1.reshape(0, 256);//256*1
			b31 = b1.reshape(0, 256);//256*1
			//cout<<f31<<endl;

			double f3mean = double(sumvec(f31) / 256);
			double b3mean = double(sumvec(b31) / 256);
			//cout << "f3mean" << f3mean << "b3mean" << b3mean << endl;
			ones = Mat::ones(cv::Size(1, 256), CV_32FC1);//256*1,CV_32FC1
			f3 = f31 - f3mean * ones;
			b3 = b31 - b3mean * ones;//256*1

			Mat zarb = f3.mul(b3);//256*1
			double zar = sumvec(zarb);
			f3f3 = f3.mul(f3);
			double zar1 = sumvec(f3f3);
			b3b3 = b3.mul(b3);
			double zar2 = sumvec(b3b3);

			double corr1 = zar / sqrt(zar1 * zar2);
			//cout << "a" << endl;
			//cout << corr1 << endl;
			if (corr1 < 0.001)
			{
				Mat s = (f1 + b1) / 2;
				double alp = 0.5;
				Mat f2 = f31.t();
				Mat b2 = b31.t();//1*256
				for (int h = 0; h < 2; h++)
				{
					// +(0.1*Mat::eye(80,80, CV_32FC1))
					Mat first = P * (alp * (P.t() * P) + (0.1 * Mat::eye(80, 80, CV_32FC1))).inv();//256*80
					Mat second(80, 1, CV_32FC1);
					second = P.t() * (f2 - b2 + alp * b2).t();
					Mat sp = first * second;//256*1
					//cout << sp << endl;
					Mat third1 = (b2 - sp.t()) * (f2 - b2).t();
					double third = double(*(float*)(third1.ptr<float>(0) + 0));
					Mat forth1 = (b2 - sp.t()) * (sp.t() - b2).t();
					double forth = double(*(float*)(forth1.ptr<float>(0) + 0));
					double alpstar = third / forth;
					//cout << "third=" << third << "forth=" << forth << endl;
					if (alpstar <= 0)
						alp = 0;
					else if (alpstar >= 1)
						alp = 1;
					else
						alp = alpstar;
				}
				//cout << alp << "  " << endl;
				if (alp > 0.3 && alp <= 0.5)
				{
					circle(f, Point(i, j), 5, Scalar(255, 0, 0));
				}
				else if (alp > 0.5 && alp <= 0.9)
				{
					circle(f, Point(i, j), 5, Scalar(0, 0, 255));
				}
				else if (alp > 0.9)
				{
					circle(f, Point(i, j), 5, Scalar(0, 255, 0));
				}
			}
		}
		cout << "i=" << i << endl;
	}

	//delete[] P1;
	cv::Mat fff(height, width, CV_8UC3);
	f.convertTo(f, CV_8UC3);
	f = convertTo3Channels(f);
	
	resize(f, fff, fff.size(), 0, 0, INTER_LINEAR);

	return fff;
}

Mat My_Detector::GetSmokeImg(Mat img)
{
	IplImage *smoke_img = &(IplImage)img;
	return smoke_process(smoke_img);
}

Mat My_Detector::GetSmokeImg_v2(Mat img)
{
	return smoke_v2(img);
}


void My_Detector::smoke_detect()
{
	const auto T1 = 40;  //15-55   
	const auto T2 = 210;  //190-245   
	const auto T3 = 245;  //200-255   
	const auto T4 = 45;
	const auto threshold = 20;

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

Mat My_Detector::GetHumanTrackImg(Mat img)
{
	return ojt->getProcessedImage(img);
}
