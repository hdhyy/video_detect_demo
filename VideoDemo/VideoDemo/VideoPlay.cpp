// VideoPlay.cpp: 实现文件
//

#include "stdafx.h"
#include "VideoDemo.h"
#include "VideoPlay.h"
#include "ImageUtils.h"
#include "afxdialogex.h"
#include "DealWithTensorFlow.h"
#include "Detection.h"
#include "PublicHeader.h"
#include "VideoHandler.h"

// VideoPlay 对话框

IMPLEMENT_DYNAMIC(VideoPlay, CDialogEx)

VideoPlay::VideoPlay(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VIDEO_PLAY_DIALOG, pParent)
{

}

VideoPlay::~VideoPlay()
{
}

void VideoPlay::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(VideoPlay, CDialogEx)
	ON_BN_CLICKED(IDC_CAM_BUTTON, &VideoPlay::OnBnClickedCamButton)
	ON_BN_CLICKED(IDC_PLAY_VIDEO_BUTTON, &VideoPlay::OnBnClickedPlayVideoButton)
	ON_BN_CLICKED(IDC_CV340_VIDEO_BUTTON, &VideoPlay::OnBnClickedCv340VideoButton)
	ON_BN_CLICKED(IDC_IMSHOW_BUTTON, &VideoPlay::OnBnClickedImshowButton)
	ON_BN_CLICKED(IDC_RESULT_BUTTON, &VideoPlay::OnBnClickedResultButton)
	ON_BN_CLICKED(IDC_HOMO_BUTTON, &VideoPlay::OnBnClickedHomoButton)
	ON_BN_CLICKED(IDC_TRACK_BUTTON, &VideoPlay::OnBnClickedTrackButton)
	ON_BN_CLICKED(IDC_MSRCR_BUTTON, &VideoPlay::OnBnClickedMsrcrButton)
	ON_BN_CLICKED(IDC_ROI_BUTTON, &VideoPlay::OnBnClickedRoiButton)
	ON_BN_CLICKED(IDC_SMOKE_TEST_BUTTON, &VideoPlay::OnBnClickedSmokeTestButton)
	ON_BN_CLICKED(IDC_FLAME_TEST_BUTTON, &VideoPlay::OnBnClickedFlameTestButton)
	ON_BN_CLICKED(IDC_GRAYWORLD_BUTTON, &VideoPlay::OnBnClickedGrayworldButton)
	ON_BN_CLICKED(IDC_LU_SHADOW_BUTTON, &VideoPlay::OnBnClickedLuShadowButton)
	ON_BN_CLICKED(IDC_MSR_BUTTON, &VideoPlay::OnBnClickedMsrButton)
	ON_BN_CLICKED(IDC_DEHAZE_BUTTON, &VideoPlay::OnBnClickedDehazeButton)
	ON_BN_CLICKED(IDC_INSECT_DETECT_BUTTON, &VideoPlay::OnBnClickedInsectDetectButton)
	ON_BN_CLICKED(IDC_BLUR_BUTTON, &VideoPlay::OnBnClickedBlurButton)
END_MESSAGE_MAP()


// VideoPlay 消息处理程序
void VideoPlay::OnBnClickedCamButton()
{
	// TODO: 在此添加控件通知处理程序代码
	cv::VideoCapture cap(0);
	cv::Mat frame;
	while (1)
	{
		cap >> frame;
		imshow("调用摄像头", frame);
		cv::waitKey(30);
	}
}


void VideoPlay::OnBnClickedPlayVideoButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CvCapture* pCapture = cvCreateFileCapture("video/demo.mp4");
	cv::Mat pFrame;
	cv::VideoCapture vcp;
	vcp.open("video/demo.mp4");
	bool f = pCapture == NULL;
	bool f2 = !vcp.isOpened();
	double fps = vcp.get(CV_CAP_PROP_FPS);
	int nFrames = (int)(vcp.get(CV_CAP_PROP_FRAME_COUNT));
	//int spf = (int)(1000 / fps);
	int u = 1;
	while (1)
	{
		vcp >> pFrame;

		if (pFrame.empty())        // 播放结束退出  
		{
			break;
		}

		imshow("VideoPlayer", pFrame);
		// 播放的过程中移动滚动条
		//printf("current pos %d\n", n_position);
		//n_position++;
		//g_slider_position = n_position;
		//setTrackbarPos("Position", "VideoPlayer", g_slider_position);
		char c = cv::waitKey((int)(1000 / fps)); // 按原来的帧率播放视频  
		if (c == 27)        // 按下Esc退出播放  
		{
			break;
		}
	}

	vcp.release();
	pFrame.release();
}


void VideoPlay::OnBnClickedCv340VideoButton()
{
	////读取视频数据，cvCreateFileCapture()通过参数设置确定要读入的avi文件，返回一个指向CvCapture结构的指针。这个结构包括了所有关于要读入avi文件的信息，其中包含状态信息。
	CvCapture *pCapture = cvCreateFileCapture("demo.mp4");
	if (pCapture == NULL)
	{
		return;//没有内容返回-1
	}
	int frames = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_COUNT);//返回视频总帧数
	int fps = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FPS);//返回视频总帧数
	IplImage* pFrame = cvQueryFrame(pCapture);//从摄像头或者文件中抓取并返回一帧
	while (pFrame)
	{
		Sleep(1000 / fps);//间隔33毫秒
		pFrame = cvQueryFrame(pCapture);//间隔33毫秒，读取下一帧
	}

	int n = 0;//用来跳转的帧数
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_POS_MSEC, n * 1000 / fps);
}


BOOL VideoPlay::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化
	cv::namedWindow("view",cv::WINDOW_AUTOSIZE);
	HWND hWnd = (HWND)cvGetWindowHandle("view");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_PIC1_STATIC)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void VideoPlay::OnBnClickedImshowButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString picPath;   //定义图片路径变量
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		NULL, this);   //选择文件对话框
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	picPath = dlg.GetPathName();  //获取图片路径
	//CString to string  使用这个方法记得字符集选用“使用多字节字符”，不然会报错
	std::string picpath = CT2A(picPath.GetBuffer());

	cv::Mat image = cv::imread(picpath);
	cv::Mat imagedst;
	//以下操作获取图形控件尺寸并以此改变图片尺寸
	CRect rect;
	GetDlgItem(IDC_PIC1_STATIC)->GetClientRect(&rect);
	cv::Rect dst(rect.left, rect.top, rect.right, rect.bottom);
	resize(image, imagedst, cv::Size(rect.Width(), rect.Height()));
	cv::imshow("view", imagedst);
}

void VideoPlay::showImage()
{
	cv::Mat imagedst;
	//以下操作获取图形控件尺寸并以此改变图片尺寸
	CRect rect;
	GetDlgItem(IDC_PIC1_STATIC)->GetClientRect(&rect);
	cv::Rect dst(rect.left, rect.top, rect.right, rect.bottom);
	resize(frame, imagedst, cv::Size(rect.Width(), rect.Height()));
	cv::imshow("view", imagedst);
}

void VideoPlay::OnBnClickedResultButton()
{
	// TODO: 在此添加控件通知处理程序代码
	DealWithTensorFlow dwtl(_T("img/1.jpg"));
	IplImage* f = dwtl.execute();
	frame = cv::cvarrToMat(f);
	showImage();
}


void VideoPlay::OnBnClickedHomoButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString picPath;   //定义图片路径变量
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		NULL, this);   //选择文件对话框
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	picPath = dlg.GetPathName();  //获取图片路径
	//CString to string  使用这个方法记得字符集选用“使用多字节字符”，不然会报错
	std::string picpath = CT2A(picPath.GetBuffer());

	cv::Mat image = cv::imread(picpath);
	ImageUtils iu(&image);
	iu.homo();
}

// 回调函数
void onChangeTrackBar(int pos, void* usrdata)
{
	// 强制类型转换
	cv::Mat src = *(cv::Mat*)(usrdata);
	cv::Mat dst;
	// 二值化
	threshold(src, dst, pos, 255, 0);

	cv::imshow("view2", dst);
}

void VideoPlay::OnBnClickedTrackButton()
{
	// TODO: 在此添加控件通知处理程序代码
	//trackbar名
	std::string trackBarName = "pos";
	//图像文件名
	std::string imgName = "Lena.jpg";
	//trackbar的值
	int posTrackBar = 0;
	//trackbar的最大值
	int maxValue = 255;

	cv::Mat img;
	//读入图像，以灰度图形式读入
	img = cv::imread(imgName, 0);
	//新建窗口
	cv::namedWindow("view2");
	imshow("view2", img);
	//创建trackbar，我们把img作为数据传进回调函数中
	createTrackbar(trackBarName, "view2", &posTrackBar, maxValue, onChangeTrackBar, &img);
	cv::waitKey();
}


void VideoPlay::OnBnClickedMsrcrButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString picPath;   //定义图片路径变量
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		NULL, this);   //选择文件对话框
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	picPath = dlg.GetPathName();  //获取图片路径
	//CString to string  使用这个方法记得字符集选用“使用多字节字符”，不然会报错
	string picpath = CT2A(picPath.GetBuffer());

	Mat image = cv::imread(picpath);
	ImageUtils iu(&image);
	frame = iu.main_msrcr();
	showImage();
}


void VideoPlay::OnBnClickedRoiButton()
{
	// TODO: 在此添加控件通知处理程序代码
	Mat src = imread("img/test.jpg");
	Mat srcROI(src, Rect(0, 0, src.cols / 2, src.rows / 2));
	imshow("view2", srcROI);

	IplImage *img = &(IplImage)src;
	cvSetImageROI(img, Rect(0, 0, src.cols / 2, src.rows / 2));
	cvResize(&(IplImage)src, img);
	cvResetImageROI(img);
	imshow("view2", cvarrToMat(img));
	
}


void VideoPlay::OnBnClickedSmokeTestButton()
{
	// TODO: 在此添加控件通知处理程序代码
	Detector *detector = new Detector();
	detector->smoke_detect();
}

#define TRAIN_MODE

#ifdef TRAIN_MODE
bool trainComplete = false;
#endif
MyVideoHandler* videoHandler = NULL;
void VideoPlay::OnBnClickedFlameTestButton()
{
	// TODO:火苗检测的测试代码
	MyVideoHandler handler("video/flame.avi");
	videoHandler = &handler;

	int ret = handler.handle();

	switch (ret) {
	case MyVideoHandler::STATUS_FLAME_DETECTED:
		cout << "Flame detected." << endl;
		break;
	case MyVideoHandler::STATUS_OPEN_CAP_FAILED:
		cout << "Open capture failed." << endl;
		break;
	case MyVideoHandler::STATUS_NO_FLAME_DETECTED:
		cout << "No flame detected." << endl;
		break;
	default:
		break;
	}
}


void VideoPlay::OnBnClickedGrayworldButton()
{
	// TODO: 在此添加控件通知处理程序代码
	Mat src = imread("img/grayworld_src.jpg");
	ImageUtils iu(&src);
	iu.gray_world(src, src);

	cvtColor(src, src, COLOR_BGR2YUV);

	vector<Mat> src_imageYUV;
	split(src, src_imageYUV);

	merge(src_imageYUV, src);
	
	cvtColor(src, src, COLOR_YUV2BGR);
	imshow("result", src);
	waitKey(0);
}


void VideoPlay::OnBnClickedLuShadowButton()
{
	// TODO: 在此添加控件通知处理程序代码
	Mat src = imread("img/grayworld_src.jpg");

	vector<Mat> bgr;
	split(src, bgr);

	Mat img_lu;
	cvtColor(src, img_lu, COLOR_BGR2YCrCb);
	vector<Mat> img_YCRCB;
	split(img_lu, img_YCRCB);

	

	// we have to find luminance of the pixel
	// here 0.0 <= source.r/source.g/source.b <= 1.0 
	// and 0.0 <= luminance <= 1.0
	Mat src_d;
	src.convertTo(src_d, CV_32F);
	src_d /= 255;
	imshow("0~1", src_d);
	Mat lum = img_YCRCB[0];
	
	Mat lum_d;
	lum.convertTo(lum_d, CV_32F);
	lum_d /= 255;
	imshow("0~1", lum_d);
	//double luminance = cv::sqrt(lumR * cv::pow(source.r, 2.0) + lumG * pow(source.g, 2.0) + lumB * pow(source.b, 2.0));

	// here highlights and and shadows are our desired filter amounts
	// highlights/shadows should be <= -1.0 and <= +1.0
	// highlights = shadows = 0.0 by default
	// you can change 0.05 and 8.0 according to your needs but okay for me
	//double highlights = 1.0, shadows = 1.0;
	//Mat h = highlights * 0.05 * (pow(8.0, lum_d) - 1.0);
	//Mat s = shadows * 0.05 * (pow(8.0, 1.0 - lum_d) - 1.0);

	//output.r = source.r + h + s;
	//output.g = source.g + h + s;
	//output.b = source.b + h + s;
}


void VideoPlay::OnBnClickedMsrButton()
{
	CString picPath;   //定义图片路径变量
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		NULL, this);   //选择文件对话框
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	picPath = dlg.GetPathName();  //获取图片路径
	//CString to string  使用这个方法记得字符集选用“使用多字节字符”，不然会报错
	string picpath = CT2A(picPath.GetBuffer());

	Mat image = cv::imread(picpath);

	ImageUtils iu(&image);
	frame = iu.main_msr();
	showImage();
}


void VideoPlay::OnBnClickedDehazeButton()
{
	CString picPath;   //定义图片路径变量
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		NULL, this);   //选择文件对话框
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	picPath = dlg.GetPathName();  //获取图片路径
	//CString to string  使用这个方法记得字符集选用“使用多字节字符”，不然会报错
	string picpath = CT2A(picPath.GetBuffer());

	Mat image = cv::imread(picpath);

	ImageUtils iu(&image);
	frame = iu.deHaze();
	showImage();
}


void VideoPlay::OnBnClickedInsectDetectButton()
{
	// TODO:image util已修改，需要增加单帧检测方法

}


void VideoPlay::OnBnClickedBlurButton()
{
	// TODO: 模糊度检测测试代码,需要修改
}
