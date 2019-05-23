#include "stdafx.h"
#include "ImageUtils.h"
#include "MSRCR.h"
using namespace std;
using namespace cv;

ImageUtils::ImageUtils(Mat *src_p)
{
	for (int i = 0; i < 3; i++)
		weight.push_back(1. / 3);
	// 由于MSRCR.h中定义了宏USE_EXTRA_SIGMA，所以此处的vector<double> sigema并没有什么意义
	sigema.push_back(30);
	sigema.push_back(150);
	sigema.push_back(300);
	src = *src_p;
}


ImageUtils::~ImageUtils()
{
}

// Normalizes a given image into a value range between 0 and 255.  
Mat norm(const Mat& src) {
	// Create and return normalized image:  
	Mat dst;
	switch (src.channels()) {
	case 1:
		cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
		break;
	case 3:
		cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
		break;
	default:
		src.copyTo(dst);
		break;
	}
	return dst;
}

void process_video()
{
	vector<double> sigema;
	vector<double> weight;
	for (int i = 0; i < 3; i++)
		weight.push_back(1. / 3);
	// 由于MSRCR.h中定义了宏USE_EXTRA_SIGMA，所以此处的vector<double> sigema并没有什么意义
	sigema.push_back(30);
	sigema.push_back(150);
	sigema.push_back(300);

	VideoCapture cap;
	cap.open("test.h264");
	
	Size size0 = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH), (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));

	auto fourcc = CV_FOURCC('D', 'I', 'V', 'X');
	VideoWriter writer("video/out.avi", fourcc, cap.get(CV_CAP_PROP_FPS), size0, true);

	if (!cap.isOpened())
		return;

	Mat frame, dst;
	Msrcr msrcr;
	int a = 100;
	while (1 && a)
	{
		cap >> frame;
		//Canny(frame, frame, 30, 100);
		msrcr.MultiScaleRetinexCR(frame, dst, weight, sigema, 100, 100);
		if (frame.empty())
			break;
		writer << dst;//等同于writer.write(frame);
		imshow("video", dst);
		if (waitKey(10) > 0)
			break;
		a--;
	}
	cout << "write end!";
	cap.release();
	writer.release();
	frame.release();
	dst.release();
	destroyAllWindows();
}

//helper function
void Morphology(const Mat &imgIn, Mat &imgOut, int morpOp = MORPH_CLOSE,
	int minThickess = 2, int shape = MORPH_ELLIPSE)
{
	int size = minThickess / 2;
	Point anchor = Point(size, size);
	Mat element = getStructuringElement(shape, Size(2 * size + 1, 2 * size + 1), anchor);
	morphologyEx(imgIn, imgOut, morpOp, element, anchor);
}

void Main_Inpaint()
{
	Mat src, blur, mask, bkMask, fgMask, dst;
	vector<vector<Point> > contours;
	src = imread("pixel-saturation1.jpg");
	// remove noise
	cv::GaussianBlur(src, blur, Size(), 2, 2);
	//CREATE A MASK FOR THE SATURATED PIXEL
	int minBrightness = 253;
	int dilateSize = 20;
	//convert to HSV
	Mat src_hsv, brightness, saturation;
	vector<Mat> hsv_planes;
	cvtColor(blur, src_hsv, COLOR_BGR2HSV);
	split(src_hsv, hsv_planes);
	brightness = hsv_planes[2];
	//get the mask
	threshold(brightness, mask, minBrightness, 255, THRESH_BINARY);
	//dialte a bit the selection
	Morphology(mask, mask, MORPH_DILATE, dilateSize);
	//INPAINTING
	double radius = 5.0;
	inpaint(src, mask, dst, radius, INPAINT_NS);
	imshow("Navier-Stokes based method", dst);

	inpaint(src, mask, dst, radius, INPAINT_TELEA);
	imshow("Method by Alexandru Telea ", dst);
	//show the selection on src
	findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
		drawContours(src, contours, i, Scalar(0, 0, 255), 2);
	imshow("Inpaint mask", src);
	waitKey(0);
}

void homomorphic(int pos, void* userdata)
{
	cv::Mat *src_t = (cv::Mat*)(userdata);
	double t2 = (double)(pos - 10) / 110;
	vector<Mat> rgb_split;
	cv::split(*src_t, rgb_split);
	Mat dst;
	for (int i = 0; i < 3; i++)
	{
		Mat original = rgb_split[i].clone();
		Mat frame_log, padded, fourier_src, spatial, reinforce_src;
		original.convertTo(frame_log, CV_32FC1);
		frame_log += 1;
		log(frame_log, frame_log);
		//将图片从空域中转至频域
		int m = frame_log.rows;
		int n = frame_log.cols;
		copyMakeBorder(frame_log, padded, 0, m - frame_log.rows, 0, n - frame_log.cols, BORDER_CONSTANT, Scalar::all(0));
		Mat image_planes[] = { Mat_<double>(padded), Mat::zeros(padded.size(), CV_32F) };
		cv::merge(image_planes, 2, fourier_src);
		dft(fourier_src, fourier_src);

		//构造同态滤波器
		Mat hu(fourier_src.size(), CV_32FC1, Scalar::all(0));
		Point center = Point(hu.rows / 2, hu.cols / 2);
		for (int i = 0; i < hu.rows; i++)
		{
			double* data = hu.ptr<double>(i);
			for (int j = 0; j < hu.cols; j++)
				data[j] = 1 / (1 + pow(sqrt(pow(center.x - i, 2) + pow((center.y - j), 2)), -t2));
		}
		Mat butterworth_channels[] = { Mat_<double>(hu), Mat::zeros(hu.size(), CV_32F) };
		cv::merge(butterworth_channels, 2, hu);

		//进行频域卷积操作，得到强化过后的频域图，将其转回空域
		cv::mulSpectrums(fourier_src, hu, fourier_src, 0);
		cv::idft(fourier_src, spatial, DFT_SCALE);

		//对图像进行还原
		cv::exp(spatial, spatial);
		vector<Mat> planes;
		cv::split(spatial, planes);
		cv::magnitude(planes[0], planes[1], reinforce_src);
		Mat temp;
		normalize(reinforce_src, temp, 0, 255, NORM_MINMAX);
		temp.convertTo(temp, CV_8UC1);
		//这里采用偏差法对图像进行还原，类似的也可以用直方图归一化，只不过我试过直方图归一化效果不是很好
		Mat mean_val, stddev_value;
		cv::meanStdDev(reinforce_src, mean_val, stddev_value);
		double min, max, minmax;
		min = mean_val.at<double>(0, 0) - 2 * stddev_value.at<double>(0, 0);
		max = mean_val.at<double>(0, 0) + 2 * stddev_value.at<double>(0, 0);
		minmax = max - min;
		for (int i = 0; i < planes[0].rows; i++)
			for (int j = 0; j < planes[0].cols; j++)
				reinforce_src.at<double>(i, j) = 255 * (reinforce_src.at<double>(i, j) - (double)min) / minmax;
		reinforce_src.convertTo(reinforce_src, CV_8UC1);
		rgb_split[i] = reinforce_src.clone();

	}
	cv::merge(rgb_split, dst);
	//下面对图像进行饱和度拉伸以及灰度线性变换以获得更加生动的图片
	cvtColor(dst, dst, COLOR_BGR2HSV);
	vector<Mat> hsv;
	split(dst, hsv);
	for (int i = 0; i < hsv[1].rows; i++)
		for (int j = 0; j < hsv[1].cols; j++)
			hsv[1].at<uchar>(i, j) = hsv[1].at<uchar>(i, j) * 4 / 3;
	for (int i = 0; i < hsv[1].rows; i++)
	{
		for (int j = 0; j < hsv[1].cols; j++)
		{
			if (hsv[2].at<uchar>(i, j) < 235)
				hsv[2].at<uchar>(i, j) = hsv[2].at<uchar>(i, j) * 45 / 50 - 40;
			else if (hsv[2].at<uchar>(i, j) > 220)
				hsv[2].at<uchar>(i, j) = (hsv[2].at<uchar>(i, j) - 151) * 55 / 180 + 200;
		}
	}
	//merge(hsv, dst);
	cvtColor(dst, dst, COLOR_HSV2BGR);
	imwrite("img/homosdst.jpg", dst);
	imshow("view2", dst);
}

Mat Fourier_Transform(Mat input)
{
	Mat result;
	return result;
}

void ImageUtils::homo()
{
	int pos = 10;
	cv::namedWindow("view2");
	imshow("view2", src);
	createTrackbar("pos", "view2", &pos, 100, homomorphic, &src);
	cv::waitKey();
}

int ImageUtils::laplacian_change()
{
	Mat image = imread("img/test.jpg");
	Mat imageLog(image.size(), CV_32FC3);
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			auto ptr = image.at<Vec3b>(i, j);
			imageLog.at<Vec3f>(i, j)[0] = (float)log(1 + ptr[0]);
			imageLog.at<Vec3f>(i, j)[1] = (float)log(1 + ptr[1]);
			imageLog.at<Vec3f>(i, j)[2] = (float)log(1 + ptr[2]);
		}
	}
	//归一化到0~255  
	normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
	//转换成8bit图像显示  
	convertScaleAbs(imageLog, imageLog);
	imwrite("img/lap.jpg", imageLog);
	imshow("src", image);
	imshow("dst", imageLog);
	waitKey(0);
	return 0;
}

IplImage *src_m, *dst_m;
int *spatialRad, *colorRad, *maxPryLevel;

void on_Meanshift(int)  //the callback function
{
	cvPyrMeanShiftFiltering(src_m, dst_m, *spatialRad, *colorRad, *maxPryLevel);  //segmentation use meanshift
	cvShowImage("dst", dst_m);   //show the segmented image
}

void call_mean(IplImage *src_m, IplImage *dst_m)
{
	src_m = cvLoadImage("img/test.jpg");   //load the picture
	CvSize size;
	size.width = src_m->width;
	size.height = src_m->height;
	dst_m = cvCreateImage(size, src_m->depth, 3);  //set the size of the dst image
	cvNamedWindow("src", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("dst", CV_WINDOW_AUTOSIZE);
	cvShowImage("src", src_m);
	cvPyrMeanShiftFiltering(src_m, dst_m, *spatialRad, *colorRad, *maxPryLevel);

	//create the trackbar
	cvCreateTrackbar("spatialRad", "dst", spatialRad, 50, on_Meanshift);
	cvCreateTrackbar("colorRad", "dst", colorRad, 60, on_Meanshift);
	cvCreateTrackbar("maxPryLevel", "dst", maxPryLevel, 5, on_Meanshift);
	cvShowImage("dst", dst_m);
	imwrite("img/mean_shift_result.jpg", cvarrToMat(dst_m));
	cvWaitKey(0);
}

void ImageUtils::mean_shift()
{
	spatialRad = &m_spatialRad;
	colorRad = &m_colorRad;
	maxPryLevel = &m_maxPryLevel;

	call_mean(src_m, dst_m);
}

void ImageUtils::eHist()
{
	///直方图均衡化
	Mat imageRGB[3];
	split(src, imageRGB);
	for (int i = 0; i < 3; i++)
	{
		equalizeHist(imageRGB[i], imageRGB[i]);
	}
	merge(imageRGB, 3, dst);
}

void ImageUtils::displayCam()
{
	///摄像头显示
	Mat image, X, I;
	VideoCapture cap(0);
	while (1)
	{
		cap >> image;
		image.convertTo(X, CV_32FC1); //转换格式
		double gamma = 4;
		pow(X, gamma, I);

		imshow("Original Image", image);
		imshow("Gamma correction image", norm(I));
		char key = waitKey(30);
		if (key == 'q')
			break;
	}
	///gamma矫正
	image.convertTo(X, CV_32FC1); //转换格式
	double gamma = 4;
	pow(X, gamma, I);
	imshow("Gamma correction image 2", norm(I));
}

void ImageUtils::highlightMask()
{
	///做高光掩码
	Mat mask;
	mask = imread("image/mask.jpg");
}

void ImageUtils::showMsrcrVideo()
{
	string path = "/home/grq/style.mp4";
    VideoCapture video(path);
    Mat frame, imgdst;
    Msrcr msrcr;
    video >> frame;
    while(!frame.empty())
    {
        video >> frame;
        imshow("SRCR", frame);
        msrcr.MultiScaleRetinex(frame, imgdst, weight, sigema, 128, 128);
        imshow("dst", imgdst);
        waitKey(20);
    }
}

Mat ImageUtils::main_msrcr()
{
	///图像预处理
	///图像处理部分
	Mat imageConvert;
	src.convertTo(imageConvert, src.type(), 1, 75);
	imwrite("img/imageConvert.jpg", imageConvert);
	//imshow("imageConvert", imageConvert);
	Msrcr msrcr;
	msrcr.MultiScaleRetinexCR(src, dst, weight, sigema, 100, 100);
	imwrite("img/dst.jpg", dst);
	return dst;
}