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
	// 由于MSRCR.h中定义了宏USE_EXTRA_SIGMA，所以此处的vector<double> sigema并没有什么意义
	sigema.push_back(30);
	sigema.push_back(150);
	sigema.push_back(300);
	src = *src_p;

	//昆虫检测的初始化代码
	Mat frame, mask;
	bgSubtractor = createBackgroundSubtractorMOG2(1000, 160, false);

	frame_count = 1;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	SimpleBlobDetector::Params params;
	//阈值控制
	params.minThreshold = 80;
	params.maxThreshold = 120;
	//像素面积大小控制
	params.filterByArea = true;
	params.minArea = 9;
	params.maxArea = 400;
	//形状（凸）
	params.filterByCircularity = false;
	params.minCircularity = 0.7;
	//形状（凹）
	params.filterByConvexity = false;
	params.minConvexity = 0.9;
	//形状（圆）
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

	//RGB三通道分离
	split(src, imageRGB);

	//求原始图像的RGB分量的均值
	double R, G, B;
	B = mean(imageRGB[0])[0];
	G = mean(imageRGB[1])[0];
	R = mean(imageRGB[2])[0];

	//需要调整的RGB分量的增益
	double KR, KG, KB;
	KB = (R + G + B) / (3 * B);
	KG = (R + G + B) / (3 * G);
	KR = (R + G + B) / (3 * R);

	//调整RGB三个通道各自的值
	imageRGB[0] = imageRGB[0] * KB;
	imageRGB[1] = imageRGB[1] * KG;
	imageRGB[2] = imageRGB[2] * KR;

	//RGB三通道图像合并
	merge(imageRGB, src);
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
	//end = clock();		//程序结束用时
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
	///图像预处理
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
	///图像处理部分
	vector<Mat> src_imageYUV;
	vector<Mat> dst_imageYUV;
	//对原图做MSRCR
	Msrcr msrcr;
	msrcr.MultiScaleRetinexCR(src, dst, weight, sigema, 100, 100);
	//原图和结果图都转换到YUV空间
	cv::cvtColor(src_yuv, src_yuv, COLOR_BGR2YCrCb);
	cv::cvtColor(dst, dst, COLOR_BGR2YCrCb);
	//分割3通道
	split(src_yuv, src_imageYUV);
	split(dst, dst_imageYUV);

	//使用结果的Y代替原图
	src_imageYUV[0] = dst_imageYUV[0];
	src_imageYUV[1] = (src_imageYUV[1] + dst_imageYUV[1]) / 2;
	src_imageYUV[2] = (src_imageYUV[2] + dst_imageYUV[2]) / 2;
	//src_imageYUV[1] += dst_imageYUV[0]*0.3;
	//src_imageYUV[2] += dst_imageYUV[0]*0.3;
	//src_imageYUV[2] *= 1.1;
	//cv::equalizeHist(src_imageYUV[0], src_imageYUV[0]);
	//合并3通道
	merge(src_imageYUV, dst);
	//将结果图转换为BGR空间
	cv::cvtColor(dst, dst, COLOR_YCrCb2BGR);
	//灰度世界白平衡
	gray_world(dst, dst);
	//imwrite("img/dst.jpg", dst);
	return dst;
}

Mat ImageUtils::main_msr()
{
	///图像预处理
	pyrDown(src, src, Size(src.cols / 2, src.rows / 2));
	///图像处理部分
	Msrcr msrcr;
	msrcr.MultiScaleRetinex(src, dst, weight, sigema, 100, 100);

	//色彩重建部分采用gimp的contrast-retinex
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
	//end = clock();		//程序结束用时
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

/*检测模糊度
 返回值为模糊度，值越大越模糊，越小越清晰，范围在0到几十，10以下相对较清晰，一般为5。
 调用时可在外部设定一个阀值，具体阈值根据实际情况决定，返回值超过阀值当作是模糊图片。
 算法所耗时间在1毫秒内
*/
Mat ImageUtils::video_blur_detect()
{
	cv::Mat img;
	cv::cvtColor(src, img, CV_BGR2GRAY); // 将输入的图片转为灰度图，使用灰度图检测模糊度

	//图片每行字节数及高  
	int width = img.cols;
	int height = img.rows;
	ushort* sobelTable = new ushort[width * height];
	memset(sobelTable, 0, width * height * sizeof(ushort));

	int i, j, mul;
	//指向图像首地址  
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
	//遍历图片  
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
	//模糊度
	float result = (float)totLen / totCount;
	delete[] sobelTable;
	sobelTable = NULL;

	string temp = "Gaussian Mixture Modeling. Progress: " + to_string(result) + "\%";
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

