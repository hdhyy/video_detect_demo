
#include "pch.h"
#include "ImageUtils.h"
#include "CvxText.hpp"

using namespace std;
using namespace cv;


ImageUtils::ImageUtils()
{
}
ImageUtils::ImageUtils(Mat* src_p)
{
	//fft.create_seeta();
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


void ImageUtils::gray_world(Mat src, Mat& dst)
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
}

//helper function
void Morphology(const Mat& imgIn, Mat& imgOut, int morpOp = MORPH_CLOSE,
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
	cv::Mat* src_t = (cv::Mat*)(userdata);
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

IplImage* src_m, * dst_m;
int* spatialRad, * colorRad, * maxPryLevel;

void on_Meanshift(int)  //the callback function
{
	cvPyrMeanShiftFiltering(src_m, dst_m, *spatialRad, *colorRad, *maxPryLevel);  //segmentation use meanshift
	cvShowImage("dst", dst_m);   //show the segmented image
}

void call_mean(IplImage* src_m, IplImage* dst_m)
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
	return Mat();
}

Mat ImageUtils::main_msrcr_ex()
{
	return Mat();
}

Mat ImageUtils::main_msr()
{
	return Mat();
}

Mat ImageUtils::deHaze()
{
	return Mat();
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
	Mat& frame = src;
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
		string temp = "Modeling" + to_string(frame_count / 4) + "\%";
		putText(frame, temp, cvPoint(30, 30),
			FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 255), 3, CV_AA);
	}
	else
	{
		putText(frame, "insect number: " + to_string(keypoints.size()), cvPoint(30, 30),
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
Mat ImageUtils::video_blur_detect(float thres)
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

	CvxText text("C:\\Program Files\\Internet Explorer\\simhei.ttf"); //指定字体

	string temp = "Status: " + to_string(result) + "\%" + (result < thres ? "Yes" : "No");
	cv::Scalar size1{ 100, 0.5, 0.1, 0 }, size2{ 50, 0, 0.1, 0 }, size3{ 50, 0, 1, 0 }, size4{ 50, 0, 0.1, 0 }; // (字体大小, 无效的, 字符间距, 无效的 }

	putText(src, temp, cvPoint(30, 30),
		FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 255), 1, CV_AA);
	if (result < thres)
	{
		text_bling++;
		if (text_bling % 2 == 0)
		{
			text.setFont(nullptr, &size1, nullptr, 0);
			text.putText(src, "警告：该摄像机套袋！", cv::Point(src.cols / 3, src.rows / 2), cv::Scalar(0, 0, 255));
		}
	}
	else
	{
		text.setFont(nullptr, &size2, nullptr, 0);
		text.putText(src, "未发现套袋情况", cv::Point(src.cols / 2.5, src.rows / 2), cv::Scalar(255, 0, 0));
	}

	//text.setFont(nullptr, &size3, nullptr, 0);
	//text.putText(src, "China", cv::Point(50, 250), cv::Scalar(0, 255, 0));

	//text.setFont(nullptr, &size4, nullptr, 0);
	//text.putText(src, "BeiJing", cv::Point(50, 300), cv::Scalar(0, 0, 255));
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


void ImageUtils::face_dr()
{
}

void ImageUtils::otk_dr()
{
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

#define my_min(x, y) ((x) < (y) ? (x) : (y))
#define my_max(x, y) ((x) > (y) ? (x) : (y))

int getM(Mat& M, Mat& M_max, const Mat& src, double& m_av, double& eps)
{
	double sum = 0;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			uchar r, g, b;
			uchar temp1, temp2;
			r = src.at<Vec3b>(i, j)[0];
			g = src.at<Vec3b>(i, j)[1];
			b = src.at<Vec3b>(i, j)[2];
			temp1 = my_min(my_min(r, g), b);
			temp2 = my_max(my_max(r, g), b);
			M.at<uchar>(i, j) = temp1;
			M_max.at<uchar>(i, j) = temp2;
			sum += temp1;
		}
	}
	m_av = sum / (src.rows * src.cols * 255);
	eps = 0.85 / (m_av);
	return 0;
}

int getAveM(Mat& M_ave, Mat& M, int r)
{
	boxFilter(M, M_ave, CV_8UC1, Size(r, r));

	return 0;
}

int getL(Mat& L, Mat& M, Mat& M_ave, double eps, double m_av)
{
	double delta = my_min(0.9, eps * m_av);
	for (int i = 0; i < M.rows; i++)
	{
		for (int j = 0; j < M.cols; j++)
		{
			L.at<uchar>(i, j) = (int)my_min(delta * M_ave.at<uchar>(i, j), M.at<uchar>(i, j));
		}
	}

	return 0;
}

int MaxMatrix(Mat& src)
{
	int temp = 0;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
			temp = my_max(src.at<uchar>(i, j), temp);
		if (temp == 255)
			return temp;
	}
	return temp;
}

double GetA(Mat& M_max, Mat& M_ave)
{
	return (MaxMatrix(M_max) + MaxMatrix(M_ave)) * 0.5;
}

int dehaze(Mat& dst, const Mat& src, Mat& L, double A)
{
	int temp, value;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			temp = L.at<uchar>(i, j);
			for (int k = 0; k < 3; k++)
			{
				value = A * (src.at<Vec3b>(i, j)[k] - temp) / (A - temp);
				if (value > 255) value = 255;
				if (value < 0) value = 0;
				dst.at<Vec3b>(i, j)[k] = value;
			}
		}
	}
	return 0;
}

Mat autocontrost(Mat& matface, int cut_limit)
{

	double HistRed[256] = { 0 };
	double HistGreen[256] = { 0 };
	double HistBlue[256] = { 0 };
	int bluemap[256] = { 0 };
	int redmap[256] = { 0 };
	int greenmap[256] = { 0 };


	for (int i = 0; i < matface.rows; i++)
	{
		for (int j = 0; j < matface.cols; j++)
		{
			int iblue = matface.at<Vec3b>(i, j)[0];
			int igreen = matface.at<Vec3b>(i, j)[1];
			int ired = matface.at<Vec3b>(i, j)[2];
			HistBlue[iblue]++;
			HistGreen[igreen]++;
			HistRed[ired]++;
		}
	}
	int PixelAmount = matface.rows * matface.cols;
	int isum = 0;
	// blue
	int iminblue = 0; int imaxblue = 0;
	for (int y = 0; y < 256; y++)
	{
		isum = isum + HistBlue[y];
		if (isum >= PixelAmount * cut_limit * 0.01)
		{
			iminblue = y;
			break;
		}
	}
	isum = 0;
	for (int y = 255; y >= 0; y--)
	{
		isum = isum + HistBlue[y];
		if (isum >= PixelAmount * cut_limit * 0.01)
		{
			imaxblue = y;
			break;
		}
	}
	//red
	isum = 0;
	int iminred = 0; int imaxred = 0;
	for (int y = 0; y < 256; y++)
	{
		isum = isum + HistRed[y];
		if (isum >= PixelAmount * cut_limit * 0.01)
		{
			iminred = y;
			break;
		}
	}
	isum = 0;
	for (int y = 255; y >= 0; y--)
	{
		isum = isum + HistRed[y];
		if (isum >= PixelAmount * cut_limit * 0.01)
		{
			imaxred = y;
			break;
		}
	}
	//green
	isum = 0;
	int imingreen = 0; int imaxgreen = 0;
	for (int y = 0; y < 256; y++)
	{
		isum = isum + HistGreen[y];
		if (isum >= PixelAmount * cut_limit * 0.01)
		{
			imingreen = y;
			break;
		}
	}
	isum = 0;
	for (int y = 255; y >= 0; y--)
	{
		isum = isum + HistGreen[y];
		if (isum >= PixelAmount * cut_limit * 0.01)
		{
			imaxgreen = y;
			break;
		}
	}

	int imin = 255; int imax = 0;

	imin = my_min(iminblue, my_min(imingreen, iminblue));
	imax = my_max(imaxblue, my_max(imaxgreen, imaxred));
	if (imin < 0) imin = 0;
	if (imax > 255) imax = 255;

	iminblue = imin;
	imingreen = imin;
	iminred = imin;

	imaxred = imax;
	imaxgreen = imax;
	imaxblue = imax;

	/////////////////
	//blue
	if (iminblue != imaxblue)
	{
		for (int y = 0; y < 256; y++)
		{
			if (y <= iminblue)
				bluemap[y] = 0;
			else if (y > imaxblue)
				bluemap[y] = 255;
			else
			{
				float ftmp = (float)(y - iminblue) / (imaxblue - iminblue);
				bluemap[y] = (int)(ftmp * 255);
			}

		}
	}
	else
	{
		for (int y = 0; y < 256; y++)
			bluemap[y] = imaxblue;
	}

	//red
	if (iminred != imaxred)
	{
		for (int y = 0; y < 256; y++)
		{
			if (y <= iminred)
				redmap[y] = 0;
			else if (y > imaxred)
				redmap[y] = 255;
			else
			{
				float ftmp = (float)(y - iminred) / (imaxred - iminred);
				redmap[y] = (int)(ftmp * 255);
			}
		}
	}
	else
	{
		for (int y = 0; y < 256; y++)
			redmap[y] = imaxred;
	}

	//green
	if (imingreen != imaxgreen)
	{
		for (int y = 0; y < 256; y++)
		{
			if (y <= imingreen)
				greenmap[y] = 0;
			else if (y > imaxgreen)
				greenmap[y] = 255;
			else
			{
				float ftmp = (float)(y - imingreen) / (imaxgreen - imingreen);
				greenmap[y] = (int)(ftmp * 255);
			}

		}
	}
	else
	{
		for (int y = 0; y < 256; y++)
			greenmap[y] = imaxgreen;
	}

	for (int i = 0; i < matface.rows; i++)
	{
		for (int j = 0; j < matface.cols; j++)
		{
			matface.at<Vec3b>(i, j)[0] = bluemap[matface.at<Vec3b>(i, j)[0]];
			matface.at<Vec3b>(i, j)[1] = greenmap[matface.at<Vec3b>(i, j)[1]];
			matface.at<Vec3b>(i, j)[2] = redmap[matface.at<Vec3b>(i, j)[2]];
		}
	}
	return matface;
}

Mat ImageUtils::fast_deHaze()
{
	double eps;
	Mat M = Mat::zeros(src.size(), CV_8UC1);
	Mat M_max = Mat::zeros(src.size(), CV_8UC1);
	Mat M_ave = Mat::zeros(src.size(), CV_8UC1);
	Mat L = Mat::zeros(src.size(), CV_8UC1);
	dst = Mat::zeros(src.size(), src.type());
	double m_av, A;
	clock_t start;
	start = clock();
	getM(M, M_max, src, m_av, eps);
	getAveM(M_ave, M, 61);
	getL(L, M, M_ave, eps, m_av);
	A = GetA(M_max, M_ave);

	dehaze(dst, src, L, A);

	double duration = (double)(clock() - start);
	cout << "Time Cost: " << duration << "ms" << endl;
	return dst;
}

