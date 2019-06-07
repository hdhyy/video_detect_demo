// VideoPlay.cpp: 实现文件
//

#include "stdafx.h"
#include "VideoDemo.h"
#include "VideoPlay.h"
#include "ImageUtils.h"
#include "afxdialogex.h"
#include "DealWithTensorFlow.h"
#include "Detection.h"

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


void VideoPlay::OnBnClickedFlameTestButton()
{
	// TODO:火苗检测的测试代码


}
