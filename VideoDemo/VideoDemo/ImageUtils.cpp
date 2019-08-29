#include "stdafx.h"
#include "ImageUtils.h"
#include "MSRCR.h"
#include "HazeRemoval.h"

using namespace std;
using namespace cv;


ImageUtils::ImageUtils()
{
}
ImageUtils::ImageUtils(Mat *src_p)
{
	for (int i = 0; i < 3; i++)
		weight.push_back(1. / 3);
	// ����MSRCR.h�ж����˺�USE_EXTRA_SIGMA�����Դ˴���vector<double> sigema��û��ʲô����
	sigema.push_back(30);
	sigema.push_back(150);
	sigema.push_back(300);
	src = *src_p;

	//������ĳ�ʼ������
	Mat frame, mask;
	bgSubtractor = createBackgroundSubtractorMOG2(1000, 160, false);

	frame_count = 1;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	SimpleBlobDetector::Params params;
	//��ֵ����
	params.minThreshold = 80;
	params.maxThreshold = 120;
	//���������С����
	params.filterByArea = true;
	params.minArea = 9;
	params.maxArea = 400;
	//��״��͹��
	params.filterByCircularity = false;
	params.minCircularity = 0.7;
	//��״������
	params.filterByConvexity = false;
	params.minConvexity = 0.9;
	//��״��Բ��
	params.filterByInertia = false;
	params.minInertiaRatio = 0.5;
	params.filterByColor = true;
	params.blobColor = 255;
	detector = cv::SimpleBlobDetector::create(params);
}


ImageUtils::~ImageUtils()
{
}


void ImageUtils::set_src(cv::Mat& src)
{
	this->src = src;
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


void ImageUtils::gray_world(Mat src, Mat &dst)
{
	vector<Mat> imageRGB;

	//RGB��ͨ������
	split(src, imageRGB);

	//��ԭʼͼ���RGB�����ľ�ֵ
	double R, G, B;
	B = mean(imageRGB[0])[0];
	G = mean(imageRGB[1])[0];
	R = mean(imageRGB[2])[0];

	//��Ҫ������RGB����������
	double KR, KG, KB;
	KB = (R + G + B) / (3 * B);
	KG = (R + G + B) / (3 * G);
	KR = (R + G + B) / (3 * R);

	//����RGB����ͨ�����Ե�ֵ
	imageRGB[0] = imageRGB[0] * KB;
	imageRGB[1] = imageRGB[1] * KG;
	imageRGB[2] = imageRGB[2] * KR;

	//RGB��ͨ��ͼ��ϲ�
	merge(imageRGB, src);
}

void process_video()
{
	vector<double> sigema;
	vector<double> weight;
	for (int i = 0; i < 3; i++)
		weight.push_back(1. / 3);
	// ����MSRCR.h�ж����˺�USE_EXTRA_SIGMA�����Դ˴���vector<double> sigema��û��ʲô����
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
		writer << dst;//��ͬ��writer.write(frame);
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
		//��ͼƬ�ӿ�����ת��Ƶ��
		int m = frame_log.rows;
		int n = frame_log.cols;
		copyMakeBorder(frame_log, padded, 0, m - frame_log.rows, 0, n - frame_log.cols, BORDER_CONSTANT, Scalar::all(0));
		Mat image_planes[] = { Mat_<double>(padded), Mat::zeros(padded.size(), CV_32F) };
		cv::merge(image_planes, 2, fourier_src);
		dft(fourier_src, fourier_src);

		//����̬ͬ�˲���
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

		//����Ƶ�����������õ�ǿ�������Ƶ��ͼ������ת�ؿ���
		cv::mulSpectrums(fourier_src, hu, fourier_src, 0);
		cv::idft(fourier_src, spatial, DFT_SCALE);

		//��ͼ����л�ԭ
		cv::exp(spatial, spatial);
		vector<Mat> planes;
		cv::split(spatial, planes);
		cv::magnitude(planes[0], planes[1], reinforce_src);
		Mat temp;
		normalize(reinforce_src, temp, 0, 255, NORM_MINMAX);
		temp.convertTo(temp, CV_8UC1);
		//�������ƫ���ͼ����л�ԭ�����Ƶ�Ҳ������ֱ��ͼ��һ����ֻ�������Թ�ֱ��ͼ��һ��Ч�����Ǻܺ�
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
	//�����ͼ����б��Ͷ������Լ��Ҷ����Ա任�Ի�ø���������ͼƬ
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
	//��һ����0~255  
	normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
	//ת����8bitͼ����ʾ  
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
	///ֱ��ͼ���⻯
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
	///����ͷ��ʾ
	Mat image, X, I;
	VideoCapture cap(0);
	while (1)
	{
		cap >> image;
		image.convertTo(X, CV_32FC1); //ת����ʽ
		double gamma = 4;
		pow(X, gamma, I);

		imshow("Original Image", image);
		imshow("Gamma correction image", norm(I));
		char key = waitKey(30);
		if (key == 'q')
			break;
	}
	///gamma����
	image.convertTo(X, CV_32FC1); //ת����ʽ
	double gamma = 4;
	pow(X, gamma, I);
	imshow("Gamma correction image 2", norm(I));
}

void ImageUtils::highlightMask()
{
	///���߹�����
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

void CLAHE_test()
{
	// READ RGB color image and convert it to Lab
	cv::Mat bgr_image = cv::imread("img/image.jpg");
	cv::Mat lab_image;
	cv::cvtColor(bgr_image, lab_image, CV_BGR2Lab);

	// Extract the L channel
	std::vector<cv::Mat> lab_planes(3);
	cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

	// apply the CLAHE algorithm to the L channel
	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(4);
	cv::Mat dst;
	clahe->apply(lab_planes[0], dst);

	// Merge the the color planes back into an Lab image
	dst.copyTo(lab_planes[0]);
	cv::merge(lab_planes, lab_image);

	// convert back to RGB
	cv::Mat image_clahe;
	cv::cvtColor(lab_image, image_clahe, CV_Lab2BGR);

	// display the results  (you might also want to see lab_planes[0] before and after).
	cv::imshow("image original", bgr_image);
	cv::imshow("image CLAHE", image_clahe);
	cv::waitKey();
}

Mat ImageUtils::main_msrcr()
{
	Msrcr msrcr;
	//clock_t start, end;
	//start = clock();
	msrcr.MultiScaleRetinexCR(src, dst, weight, sigema, 100, 100);
	//end = clock();		//���������ʱ
	//double endtime = (double)(end - start) / CLOCKS_PER_SEC;
	//string t_s = to_string(endtime);
	//putText(dst, t_s,Point(50, 50), cv::FONT_HERSHEY_PLAIN, 2, Scalar(255,0,0));
	//imshow("in", dst);
	gray_world(dst, dst);
	imwrite("img/dst.jpg", dst);
	return dst;
}

Mat ImageUtils::main_msrcr_ex()
{
	///ͼ��Ԥ����
	//Mat imageConvert;
	//src.convertTo(imageConvert, src.type(), 1, 75);
	//imwrite("img/imageConvert.jpg", imageConvert);
	//imshow("imageConvert", imageConvert);
	Mat src_hsv;
	src.copyTo(src_hsv);
	cv::cvtColor(src_hsv, src_hsv, COLOR_BGR2HSV);

	vector<Mat> src_imageHSV;
	split(src_hsv, src_imageHSV);

	//Mat image_ostu;
	//cv::threshold(src_imageHSV[2], image_ostu, 200, 255, CV_THRESH_BINARY);
	int nr = src.rows, nc = src.cols;
	Mat mask = src_imageHSV[2] > 200;
	cv::add(src_imageHSV[0], src_imageHSV[1], src_imageHSV[2], mask);

	merge(src_imageHSV, src_hsv);
	cvtColor(src_hsv, src_hsv, COLOR_HSV2BGR);

	Mat src_yuv;
	src.copyTo(src_yuv);
	///ͼ������
	vector<Mat> src_imageYUV;
	vector<Mat> dst_imageYUV;
	//��ԭͼ��MSRCR
	Msrcr msrcr;
	msrcr.MultiScaleRetinexCR(src, dst, weight, sigema, 100, 100);
	//ԭͼ�ͽ��ͼ��ת����YUV�ռ�
	cv::cvtColor(src_yuv, src_yuv, COLOR_BGR2YCrCb);
	cv::cvtColor(dst, dst, COLOR_BGR2YCrCb);
	//�ָ�3ͨ��
	split(src_yuv, src_imageYUV);
	split(dst, dst_imageYUV);

	//ʹ�ý����Y����ԭͼ
	src_imageYUV[0] = dst_imageYUV[0];
	src_imageYUV[1] = (src_imageYUV[1] + dst_imageYUV[1]) / 2;
	src_imageYUV[2] = (src_imageYUV[2] + dst_imageYUV[2]) / 2;
	//src_imageYUV[1] += dst_imageYUV[0]*0.3;
	//src_imageYUV[2] += dst_imageYUV[0]*0.3;
	//src_imageYUV[2] *= 1.1;
	//cv::equalizeHist(src_imageYUV[0], src_imageYUV[0]);
	//�ϲ�3ͨ��
	merge(src_imageYUV, dst);
	//�����ͼת��ΪBGR�ռ�
	cv::cvtColor(dst, dst, COLOR_YCrCb2BGR);
	//�Ҷ������ƽ��
	gray_world(dst, dst);
	//imwrite("img/dst.jpg", dst);
	return dst;
}

Mat ImageUtils::main_msr()
{
	///ͼ��Ԥ����
	pyrDown(src, src, Size(src.cols / 2, src.rows / 2));
	///ͼ������
	Msrcr msrcr;
	msrcr.MultiScaleRetinex(src, dst, weight, sigema, 100, 100);

	//ɫ���ؽ����ֲ���gimp��contrast-retinex
	Mat src_yuv;
	src.copyTo(src_yuv);
	cvtColor(src_yuv, src_yuv, COLOR_BGR2YCrCb);
	cvtColor(dst, dst, COLOR_BGR2YCrCb);
	vector<Mat> src_imageYUV;
	vector<Mat> dst_imageYUV;
	split(src_yuv, src_imageYUV);
	split(dst, dst_imageYUV);
	src_imageYUV[0] = dst_imageYUV[0];
	//dst_imageYUV[1] = (src_imageYUV[1]*3 + dst_imageYUV[1]) / 4;
	//dst_imageYUV[2] = (src_imageYUV[2]*3 + dst_imageYUV[2]) / 4;

	merge(src_imageYUV, dst);
	cvtColor(dst, dst, COLOR_YCrCb2BGR);
	pyrUp(dst, dst, Size(dst.cols * 2, dst.rows * 2));
	gray_world(dst, dst);
	return dst;
}

Mat ImageUtils::deHaze()
{
	//clock_t start, end;
	//start = clock();
	HazeRemoval(src, dst);
	//end = clock();		//���������ʱ
	//double endtime = (double)(end - start) / CLOCKS_PER_SEC;
	return dst;
}

int filter_contour_area(vector<vector<Point>> contours, int area = 5)
{
	int count = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		if ((contourArea(contours[i], false)) > area)
		{
			count++;
		}
	}
	return count;
}

cv::Mat ImageUtils::insect_detect()
{
	++frame_count;
	Mat mask;
	Mat &frame = src;
	bgSubtractor->apply(frame, mask, 0.001);
	Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element2 = getStructuringElement(MORPH_RECT, Size(11, 11));
	Mat mask_tmp = mask.clone();
	morphologyEx(mask_tmp, mask_tmp, MORPH_OPEN, element1);
	morphologyEx(mask_tmp, mask_tmp, MORPH_CLOSE, element2);
	morphologyEx(mask_tmp, mask_tmp, MORPH_CLOSE, element2);
	morphologyEx(mask_tmp, mask_tmp, MORPH_CLOSE, element2);
	//dilate(mask_tmp, mask_tmp, element1);
	/*Mat mask_tmp = mask.clone();
	findContours(mask_tmp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);*/
	vector<KeyPoint> keypoints;
	detector->detect(mask_tmp, keypoints);
	if (frame_count < 400)
	{
		string temp = "Gaussian Mixture Modeling. Progress: " + to_string(frame_count / 4) + "\%";
		putText(frame, temp, cvPoint(30, 30),
			FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 255), 3, CV_AA);
	}
	else
	{
		putText(frame, "Detected active insect number: " + to_string(keypoints.size()), cvPoint(30, 30),
			FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 255), 3, CV_AA);

		drawKeypoints(frame, keypoints, frame, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	}

	//count = filter_contour_area(contours, 5);
	/*if (frame_count > 400)
		cout << "insect number: " << count << endl;*/

	return frame;
}

/*���ģ����
 ����ֵΪģ���ȣ�ֵԽ��Խģ����ԽСԽ��������Χ��0����ʮ��10������Խ�������һ��Ϊ5��
 ����ʱ�����ⲿ�趨һ����ֵ��������ֵ����ʵ���������������ֵ������ֵ������ģ��ͼƬ��
 �㷨����ʱ����1������
*/
Mat ImageUtils::video_blur_detect()
{
	cv::Mat img;
	cv::cvtColor(src, img, CV_BGR2GRAY); // �������ͼƬתΪ�Ҷ�ͼ��ʹ�ûҶ�ͼ���ģ����

	//ͼƬÿ���ֽ�������  
	int width = img.cols;
	int height = img.rows;
	ushort* sobelTable = new ushort[width * height];
	memset(sobelTable, 0, width * height * sizeof(ushort));

	int i, j, mul;
	//ָ��ͼ���׵�ַ  
	uchar* udata = img.data;
	for (i = 1, mul = i * width; i < height - 1; i++, mul += width)
		for (j = 1; j < width - 1; j++)

			sobelTable[mul + j] = abs(udata[mul + j - width - 1] + 2 * udata[mul + j - 1] + udata[mul + j - 1 + width] - \
				udata[mul + j + 1 - width] - 2 * udata[mul + j + 1] - udata[mul + j + width + 1]);

	for (i = 1, mul = i * width; i < height - 1; i++, mul += width)
		for (j = 1; j < width - 1; j++)
			if (sobelTable[mul + j] < 50 || sobelTable[mul + j] <= sobelTable[mul + j - 1] || \
				sobelTable[mul + j] <= sobelTable[mul + j + 1]) sobelTable[mul + j] = 0;

	int totLen = 0;
	int totCount = 1;

	uchar suddenThre = 50;
	uchar sameThre = 3;
	//����ͼƬ  
	for (i = 1, mul = i * width; i < height - 1; i++, mul += width)
	{
		for (j = 1; j < width - 1; j++)
		{
			if (sobelTable[mul + j])
			{
				int   count = 0;
				uchar tmpThre = 5;
				uchar max = udata[mul + j] > udata[mul + j - 1] ? 0 : 1;

				for (int t = j; t > 0; t--)
				{
					count++;
					if (abs(udata[mul + t] - udata[mul + t - 1]) > suddenThre)
						break;

					if (max && udata[mul + t] > udata[mul + t - 1])
						break;

					if (!max && udata[mul + t] < udata[mul + t - 1])
						break;

					int tmp = 0;
					for (int s = t; s > 0; s--)
					{
						if (abs(udata[mul + t] - udata[mul + s]) < sameThre)
						{
							tmp++;
							if (tmp > tmpThre) break;
						}
						else break;
					}

					if (tmp > tmpThre) break;
				}

				max = udata[mul + j] > udata[mul + j + 1] ? 0 : 1;

				for (int t = j; t < width; t++)
				{
					count++;
					if (abs(udata[mul + t] - udata[mul + t + 1]) > suddenThre)
						break;

					if (max && udata[mul + t] > udata[mul + t + 1])
						break;

					if (!max && udata[mul + t] < udata[mul + t + 1])
						break;

					int tmp = 0;
					for (int s = t; s < width; s++)
					{
						if (abs(udata[mul + t] - udata[mul + s]) < sameThre)
						{
							tmp++;
							if (tmp > tmpThre) break;
						}
						else break;
					}

					if (tmp > tmpThre) break;
				}
				count--;

				totCount++;
				totLen += count;
			}
		}
	}
	//ģ����
	float result = (float)totLen / totCount;
	delete[] sobelTable;
	sobelTable = NULL;

	string temp = "Current Status: " + to_string(result) + "\%" + (result < 3.5?"Yes":"No");
	putText(src, temp, cvPoint(30, 30),
		FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 255), 1, CV_AA);

	return src;
}

void InitConsole()
{
	int nRet = 0;
	FILE* fp;
	AllocConsole();
	nRet = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	fp = _fdopen(nRet, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
}

Mat ImageUtils::face_detect()
{
	return fft.get_detect_result(SEETAFACE2, &src);
}

void ImageUtils::face_dr()
{
	fft.play_demo();
}

void ImageUtils::otk_dr()
{
	otk.play_demo();
}


string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}
bool isU(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  return true; break;
	case CV_8S:  return false; break;
	case CV_16U: return true; break;
	case CV_16S: return false; break;
	case CV_32S: return false; break;
	case CV_32F: return false;  break;
	case CV_64F: return false; break;
	default:     return false; break;
	}
}
bool isF(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  return false; break;
	case CV_8S:  return false; break;
	case CV_16U: return false; break;
	case CV_16S: return false; break;
	case CV_32S: return false; break;
	case CV_32F: return true;  break;
	case CV_64F: return true; break;
	default:     return false; break;
	}
}
Mat U2F(Mat src)
{
	// convert unsigned char (0-255) image to float (0-1) image
	Mat dst;
	src.convertTo(dst, CV_32F, 1 / 255.0);
	return dst;
}
Mat F2U(Mat src)
{
	// convert float (0-1) image to unsigned char (0-255) image
	Mat dst = src.clone();
	src.convertTo(dst, CV_8U, 255);
	return dst;
}
void print_size(Mat src)
{
	// print height width channels
	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();
	cout << type2str(src.type()) << endl;
	cout << "h: " << size_h << " w: " << size_w << " z: " << size_z << endl;
	return;
}
double min(Mat Y)
{
	// return min element in an image
	double min, max;
	minMaxIdx(Y, &min, &max);
	return min;
}
double max(Mat Y)
{
	// return max element in an image
	double min, max;
	minMaxIdx(Y, &min, &max);
	return max;
}
void arg_min(Mat Y, int idx[2])
{
	minMaxIdx(Y, 0, 0, idx, 0);
	return;
}
void arg_max(Mat Y, int idx[2])
{
	minMaxIdx(Y, 0, 0, 0, idx);
	return;
}
Mat reduce_min(Mat in)
{
	Mat src = in.clone();
	if (!isF(src.type()))
	{
		src = U2F(src);
	}
	// reduce min of an image over the third dimension
	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();

	vector<Mat> channels(size_z);

	split(src, channels);
	Mat minmat(size_h, size_w, CV_32F, Scalar(FLT_MAX));
	for (int z = 0; z < size_z; ++z)
	{
		min(channels[z], minmat, minmat);
	}
	return minmat;
}
Mat reduce_max(Mat in)
{
	Mat src = in.clone();

	// reduce min of an image over the third dimension
	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();

	vector<Mat> channels(size_z);
	split(src, channels);
	Mat maxmat;
	if (isF(src.type()))
	{
		maxmat = Mat(size_h, size_w, CV_32F, Scalar(FLT_MIN));
	}
	else if (isU(src.type()))
	{
		maxmat = Mat(size_h, size_w, CV_8U, Scalar(0));
	}

	for (int z = 0; z < size_z; ++z)
	{
		max(channels[z], maxmat, maxmat);
	}

	return maxmat;
}
Mat reduce_mean(Mat in)
{
	Mat src = in.clone();
	if (!isF(src.type()))
	{
		src = U2F(src);
	}
	// reduce mean of an image over the third dimension
	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();

	vector<Mat> channels(size_z);

	split(src, channels);
	Mat summat(size_h, size_w, CV_32F, Scalar(0));
	for (int z = 0; z < size_z; ++z)
	{
		summat = summat + channels[z];
	}
	return summat / size_z;
}
Mat cumsum(Mat src)
{
	Mat dst = src.clone();
	if (!isF(dst.type()))
	{
		dst = U2F(dst);
	}
	// cumsum of Mat array
	int size_h = dst.size[0];
	int size_w = dst.size[1];

	for (int i = 1; i < size_h; ++i)
	{
		dst.at<float>(i) += dst.at<float>(i - 1);
	}
	return dst;
}
template<typename T>
vector<float> cumsum_vec(vector<T> src)
{
	vector<float> dst(src.size(), 0);
	for (int i = 0; i < src.size(); ++i)
	{
		dst[i] = (float)src[i];
	}
	for (int i = 1; i < src.size(); ++i)
	{
		dst[i] += dst[i - 1];
	}
	return dst;
}
Mat myCalcHist(Mat V1, int bins = 100)
{
	Mat ht;
	int channels[] = { 0 };
	int histSize[] = { bins };
	float granges[] = { 0, 255 };
	const float* ranges[] = { granges };
	calcHist(&V1, 1, channels, Mat(), ht, 1, histSize, ranges, true, false);
	return ht;

}
bool all(Mat src)
{
	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();
	bool result = true;
	for (int i = 0; i < size_h; i++)
	{
		for (int j = 0; j < size_w; j++)
		{
			for (int z = 0; z < size_z; z++)
			{
				if (src.at<uchar>(i, j, z) == 0)
					result = false;
			}
		}
	}
	return result;
}
bool all(vector<bool> mask)
{
	bool result = true;
	for (int i = 0; i < mask.size(); i++)
	{
		if (!mask[i])
			result = false;
	}
	return result;
}

template<typename T>
vector<T> abs(vector<T> src)
{
	vector<T> dst(src.begin(), src.end());
	for (int i = 0; i < src.size(); i++)
	{
		dst[i] = abs(src[i]);
	}
	return dst;
}
Mat masked_array(Mat src, Mat mask, int padd = 255)
{
	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();
	Mat result = src.clone();
	for (int i = 0; i < size_h; i++)
	{
		for (int j = 0; j < size_w; j++)
		{
			for (int z = 0; z < size_z; z++)
			{
				if (!(mask.at<uchar>(i, j, z) == 0))
				{
					result.at<int>(i, j, z) = padd;
				}

				else
				{
					result.at<int>(i, j, z) = src.at<int>(i, j, z);
				}

			}
		}
	}
	return result;
}
template<typename T>
vector<T> masked_array(vector<T> src, vector<bool> mask, T padd)
{
	vector<T> result(src.size(), padd);
	for (int i = 0; i < src.size(); i++)
	{
		if (!(mask[i] == false))
		{
			result[i] = padd;
		}

		else
		{
			result[i] = src[i];
		}
	}
	return result;
}
template <typename T, typename Compare>
std::vector<std::size_t> sort_permutation(
	const std::vector<T>& vec,
	Compare& compare)
{
	std::vector<std::size_t> p(vec.size());
	std::iota(p.begin(), p.end(), 0);
	std::sort(p.begin(), p.end(),
		[&](std::size_t i, std::size_t j) { return compare(vec[i], vec[j]); });
	return p;
}
template <typename T>
std::vector<T> apply_permutation(
	const std::vector<T>& vec,
	const std::vector<std::size_t>& p)
{
	std::vector<T> sorted_vec(vec.size());
	std::transform(p.begin(), p.end(), sorted_vec.begin(),
		[&](std::size_t i) { return vec[i]; });
	return sorted_vec;
}

void unique(vector<int> src, vector<int>& uni, vector<int>& idx, vector<int>& count)
{
	/*int size_h = src.size();
	idx = vector<int>(size_h,0);

	for (int i = 0; i < size_h; i++)
	{
		float value = src[i];
		if (find(uni.begin(), uni.end(), value) == uni.end())
		{
			uni.push_back(value);
		}
	}

	sort(uni.begin(), uni.end());

	count = vector<int>(uni.size(),0);
	for (int i = 0; i < size_h; i++)
	{
		float value = src[i];
		for (int j = 0; j < uni.size(); j++)
		{
			if (value == uni[j])
			{
				count[j]++;
				idx[i] = j;
			}
		}
	}*/
	idx = vector<int>(src.size(), 0);
	int hash[256] = { 0 };
	for (int i = 0; i < src.size(); i++)
	{
		hash[src[i]]++;
	}
	int k = 0;
	int new_idx[256];
	for (int i = 0; i < 256; i++)
	{
		if (hash[i] != 0)
		{
			uni.push_back(i);
			count.push_back(hash[i]);
			new_idx[i] = k;
			k++;
		}
	}
	for (int i = 0; i < src.size(); i++)
	{
		idx[i] = new_idx[src[i]];
	}

	return;
}
template<typename T>
void print(std::vector <T> const& a) {
	std::cout << "The vector elements are : ";

	for (int i = 0; i < a.size(); i++)
		std::cout << a.at(i) << ' ';
	std::cout << "\n" << endl;
}
template<typename T>
std::vector<T> slice_vec(std::vector<T> const& v, int m, int n)
{
	auto first = v.cbegin() + m;
	auto last = v.cbegin() + n + 1;

	std::vector<T> vec(first, last);
	return vec;
}


Mat zmMinFilterGray(Mat src, int r = 7)
{
	Mat dst;
	erode(src, dst, getStructuringElement(MORPH_RECT, Size(2 * r + 1, 2 * r + 1)));
	//imshow("erode", dst);
	//waitKey();

	return dst;
}
Mat guidedfilter(Mat I, Mat p, int r, float eps)
{
	Mat m_I, m_p, m_Ip, m_II, m_a, m_b;
	boxFilter(I, m_I, -1, { r, r });
	boxFilter(p, m_p, -1, { r, r });
	boxFilter(I.mul(p), m_Ip, -1, { r, r });
	Mat cov_Ip = m_Ip - m_I.mul(m_p);
	boxFilter(I.mul(I), m_II, -1, { r, r });
	Mat var_I = m_II - m_I.mul(m_I);
	Mat a = cov_Ip / (var_I + eps);
	Mat b = m_p - a.mul(m_I);
	boxFilter(a, m_a, -1, { r, r });
	boxFilter(b, m_b, -1, { r, r });
	return m_a.mul(I) + m_b;
}
int getV1(Mat m, int r, float eps, float w, float maxV1, Mat& V1, float& A)
{
	V1 = reduce_min(m);
	//imshow("dark", V1);
	//waitKey();

	V1 = guidedfilter(V1, zmMinFilterGray(V1, 7), r, eps);
	//imshow("filter", V1);
	//waitKey();
	int bins = 100;
	Mat ht = myCalcHist(V1, bins);

	ht.convertTo(ht, CV_32F, V1.size[0] * V1.size[1]);
	Mat d = cumsum(ht);
	int lmax = bins - 1;
	for (; lmax > 0; lmax--)
	{
		if (d.at<float>(lmax) < 0.999)
			break;
	}
	Mat avg = reduce_mean(m);
	int size_h = m.size[0];
	int size_w = m.size[1];

	A = -1;                 //negative inf
	for (int i = 0; i < size_h; ++i)
	{
		for (int j = 0; j < size_w; ++j)
		{
			if (V1.at<float>(i, j) >= lmax / bins)
				if (avg.at<float>(i, j) > A)
					A = avg.at<float>(i, j);
		}
	}
	min(V1 * w, maxV1, V1);
	return 0;
}
Mat ImageUtils::deHaze_chai(int r, float eps, float w, float maxV1)
{
	if (!isF(src.type()))
	{
		src = U2F(src);
	}
	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();
	dst = src.clone();
	Mat V1;
	float A;
	getV1(src, r, eps, w, maxV1, V1, A);
	//waitKey(0);
	vector<Mat> channels_Y(size_z);
	vector<Mat> channels_m(size_z);
	split(src, channels_m);
	for (int k = 0; k < size_z; ++k)
	{
		channels_Y[k] = (channels_m[k] - V1) / (1 - V1 / A);
	}
	merge(channels_Y, dst);
	max(dst, 0, dst); //clip 
	min(dst, 1, dst); //clip
	dst = F2U(dst);
	return dst;
}
void normalize(Mat& src)
{
	src.convertTo(src, CV_32F, 1.0 / max(src));
	src.convertTo(src, CV_8U, 255);
}
template<typename T>
int find_nearest_above(vector<T> my_array, T target)
{
	vector<T> diff(my_array.size(), 0);
	for (int i = 0; i < my_array.size(); i++)
	{
		diff[i] = my_array[i] - target;
	}
	vector<bool> mask(my_array.size(), false);
	for (int i = 0; i < diff.size(); i++)
	{
		mask[i] = diff[i] <= -1;
	}
	if (all(mask))
	{
		vector<T> abs_diff = abs(diff);
		vector<T>::iterator iter = min_element(abs_diff.begin(), abs_diff.end());
		int c = iter - abs_diff.begin();
		return c;
	}

	vector<T> masked_diff = masked_array(diff, mask, 255);


	vector<T>::iterator iter = min_element(masked_diff.begin(), masked_diff.end());
	int c = iter - masked_diff.begin();
	return c;
}
Mat hist_match(Mat original, Mat specified)
{

	int ori_h = original.size[0];
	int ori_w = original.size[1];
	int sp_h = specified.size[0];
	int sp_w = specified.size[1];
	vector<int> ori_array(ori_h * ori_w, 0), sp_array(sp_h * sp_w, 0);
	for (int i = 0; i < ori_h; i++)
	{
		for (int j = 0; j < ori_w; j++)
		{
			ori_array[i * ori_w + j] = original.at<uchar>(i, j);
		}
	}
	for (int i = 0; i < sp_h; i++)
	{
		for (int j = 0; j < sp_w; j++)
		{
			sp_array[i * sp_w + j] = specified.at<uchar>(i, j);
		}
	}
	vector<int> s_values, bin_idx, s_counts, t_values, t_counts;
	unique(ori_array, s_values, bin_idx, s_counts);
	unique(sp_array, t_values, vector<int>(), t_counts);
	/*print(s_values);
	vector<int> sliced = slice_vec(bin_idx, 0, 10);
	print(sliced);
	print(s_counts);*/
	vector<float> s_quantities = cumsum_vec(s_counts);
	vector<float> t_quantities = cumsum_vec(t_counts);
	for (int i = 0; i < s_quantities.size(); i++)
	{
		s_quantities[i] /= s_quantities[s_quantities.size() - 1];
	}
	for (int i = 0; i < t_quantities.size(); i++)
	{
		t_quantities[i] /= t_quantities[t_quantities.size() - 1];
	}
	vector<int> sour(s_quantities.size(), 0), temp(t_quantities.size(), 0);
	for (int i = 0; i < s_quantities.size(); i++)
	{
		sour[i] = (int)(s_quantities[i] * 255);
	}
	for (int i = 0; i < t_quantities.size(); i++)
	{
		temp[i] = (int)(t_quantities[i] * 255);
	}
	vector<int> b;
	for (int i = 0; i < sour.size(); i++)
	{
		b.push_back(find_nearest_above(temp, sour[i]));
	}
	/*cout << "b[i]" << endl;
	for (int i = 0; i < b.size(); i++)
	{
		cout << b[i] << endl;
	}*/
	Mat dst = original.clone();
	int k = 0;
	for (int i = 0; i < ori_h; i++)
	{
		for (int j = 0; j < ori_w; j++)
		{
			dst.at<uchar>(i, j) = b[bin_idx[k]];
			k++;
		}
	}

	return dst;
}
Mat bright_hist_match(Mat src)
{
	if (!isU(src.type()))
	{
		src = F2U(src);
	}

	int size_h = src.size[0];
	int size_w = src.size[1];
	int size_z = src.channels();
	Mat dst;
	Mat equalized;

	vector<Mat> channels_src(size_z);
	vector<Mat> channels_tmp(size_z);
	vector<Mat> channels_dst(size_z);
	split(src, channels_src);


	for (int i = 0; i < size_z; i++)
	{
		equalizeHist(channels_src[i], channels_tmp[i]);
	}
	merge(channels_tmp, equalized);
	Mat bright_prior = reduce_max(equalized);

	for (int i = 0; i < size_z; i++)
	{
		channels_dst[i] = hist_match(channels_tmp[i], bright_prior);
	}
	merge(channels_dst, dst);
	print_size(dst);
	normalize(dst);

	return dst;
}
void ImageUtils::hist_match_test()
{
	clock_t start, end;
	Mat img = imread("img/dark_test.jpg");
	start = clock();
	img = bright_hist_match(img);
	end = clock();
	double cost = (double)(end - start) / CLOCKS_PER_SEC;
	cout << cost << "s" << endl;
	cout << img.size[1] << "x" << img.size[0] << endl;
	cout << type2str(img.type()) << endl;
	//cout << img << endl;

	imshow("fuck", img);
	waitKey();
	imwrite("img/fuck.png", img);
	waitKey();
}
cv::Mat ImageUtils::get_hist_match()
{
	dst = bright_hist_match(src);
	return dst;
}

cv::Mat ImageUtils::brighten()
{
	CV_Assert(src.type() == CV_8UC3 || src.type() == CV_8UC1);

	cv::Mat src_inverse = ~src;

	cv::Mat temp;
	fastHazeRemoval(src_inverse, temp);

	dst = ~temp;
	return dst;
}

void ImageUtils::fastHazeRemoval(const cv::Mat& src, cv::Mat& dst)
{
	CV_Assert(src.type() == CV_8UC3 || src.type() == CV_8UC1);

	if (src.channels() == 1)
	{
		fastHazeRemoval_1Channel(src, dst);
	}
	else
	{
		fastHazeRemoval_3Channel(src, dst);
	}
}

void ImageUtils::fastHazeRemoval_1Channel(const cv::Mat& src, cv::Mat& dst)
{
	CV_Assert(src.type() == CV_8UC1);

	// step 1. input H(x)
	const cv::Mat& H = src;

	// step 2. calc M(x)
	const cv::Mat& M = H;

	// step 3. calc M_ave(x)
	cv::Mat M_ave;
	const int radius = std::max(50, std::max(H.rows, H.cols) / 20); // should not be too small, or there will be a halo artifact 
	cv::boxFilter(M, M_ave, -1, cv::Size(2 * radius + 1, 2 * radius + 1));

	// step 4. calc m_av
	const float m_av = float(cv::mean(M)[0] / 255.0);

	// step 5. calc L(x)
	const float p = 1.0f - m_av + 0.9f; // a simple parameter selection strategy, for reference only
	const float coeff = std::min(p * m_av, 0.9f);
	cv::Mat L(H.size(), CV_32FC1);
	for (int y = 0; y < L.rows; ++y)
	{
		const uchar* M_line = M.ptr<uchar>(y);
		const uchar* M_ave_line = M_ave.ptr<uchar>(y);
		float* L_line = L.ptr<float>(y);
		for (int x = 0; x < L.cols; ++x)
		{
			L_line[x] = std::min(coeff * M_ave_line[x], float(M_line[x]));
		}
	}

	// step 6. calc A
	double max_H = 0.0;
	cv::minMaxLoc(H, nullptr, &max_H);
	double max_M_ave = 0.0;
	cv::minMaxLoc(M_ave, nullptr, &max_M_ave);
	const float A = 0.5f * float(max_H) + 0.5f * float(max_M_ave);

	// step 7. get F(x)
	cv::Mat F(H.size(), CV_8UC1);
	for (int y = 0; y < F.rows; ++y)
	{
		const uchar* H_line = H.ptr<uchar>(y);
		const float* L_line = L.ptr<float>(y);
		uchar* F_line = F.ptr<uchar>(y);
		for (int x = 0; x < F.cols; ++x)
		{
			const float l = L_line[x];
			const float factor = 1.0f / (1.0f - l / A);
			F_line[x] = cv::saturate_cast<uchar>((float(H_line[x]) - l) * factor);
		}
	}

	dst = F;
}

void ImageUtils::fastHazeRemoval_3Channel(const cv::Mat& src, cv::Mat& dst)
{
	CV_Assert(src.type() == CV_8UC3);

	// step 1. input H(x)
	const cv::Mat& H = src;

	// step 2. calc M(x)
	cv::Mat M(H.size(), CV_8UC1);
	uchar max_H = 0; // used in step 6
	for (int y = 0; y < M.rows; ++y)
	{
		const cv::Vec3b* H_line = H.ptr<cv::Vec3b>(y);
		uchar* M_line = M.ptr<uchar>(y);
		for (int x = 0; x < M.cols; ++x)
		{
			const cv::Vec3b& h = H_line[x];
			M_line[x] = std::min(h[2], std::min(h[0], h[1]));
			max_H = std::max(std::max(h[0], h[1]), std::max(h[2], max_H));
		}
	}

	// step 3. calc M_ave(x)
	cv::Mat M_ave;
	const int radius = std::max(50, std::max(H.rows, H.cols) / 20); // should not be too small, or there will be a halo artifact 
	cv::boxFilter(M, M_ave, -1, cv::Size(2 * radius + 1, 2 * radius + 1));

	// step 4. calc m_av
	const float m_av = float(cv::mean(M)[0] / 255.0);

	// step 5. calc L(x)
	const float p = 1.0f - m_av + 0.9f; // a simple parameter selection strategy, for reference only
	const float coeff = std::min(p * m_av, 0.9f);
	cv::Mat L(H.size(), CV_32FC1);
	for (int y = 0; y < L.rows; ++y)
	{
		const uchar* M_line = M.ptr<uchar>(y);
		const uchar* M_ave_line = M_ave.ptr<uchar>(y);
		float* L_line = L.ptr<float>(y);
		for (int x = 0; x < L.cols; ++x)
		{
			L_line[x] = std::min(coeff * M_ave_line[x], float(M_line[x]));
		}
	}

	// step 6. calc A
	double max_M_ave = 0.0;
	cv::minMaxLoc(M_ave, nullptr, &max_M_ave);
	const float A = 0.5f * max_H + 0.5f * float(max_M_ave);

	// step 7. get F(x)
	cv::Mat F(H.size(), CV_8UC3);
	for (int y = 0; y < F.rows; ++y)
	{
		const cv::Vec3b* H_line = H.ptr<cv::Vec3b>(y);
		const float* L_line = L.ptr<float>(y);
		cv::Vec3b* F_line = F.ptr<cv::Vec3b>(y);
		for (int x = 0; x < F.cols; ++x)
		{
			const cv::Vec3b& h = H_line[x];
			const float l = L_line[x];
			cv::Vec3b& f = F_line[x];
			const float factor = 1.0f / (1.0f - l / A);
			f[0] = cv::saturate_cast<uchar>((float(h[0]) - l) * factor);
			f[1] = cv::saturate_cast<uchar>((float(h[1]) - l) * factor);
			f[2] = cv::saturate_cast<uchar>((float(h[2]) - l) * factor);
		}
	}

	dst = F;
}

