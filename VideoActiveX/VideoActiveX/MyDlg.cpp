// MyDlg.cpp: 实现文件

/*
来自阿里云的 NTP 服务器：【7个】
ntp1.aliyun.com
ntp2.aliyun.com
ntp3.aliyun.com
ntp4.aliyun.com
ntp5.aliyun.com
ntp6.aliyun.com
ntp7.aliyun.com
*/

#include "pch.h"
#include "VideoActiveX.h"
#include "MyDlg.h"
#include "afxdialogex.h"
#include "ImageUtils.h"
#include "My_Detector.h"

#include "winsock2.h"
#include "WS2tcpip.h"
#pragma comment(lib, "WS2_32.lib")  // 显式连接套接字库

#define TEXTURE 0
#define ANGPOINT 1
#define HELMAT 2
#define NIGHTENHANCE 3
#define HUMANTRACE 4
#define SMOKING 5
#define FLAME 6
#define INSECT_DETECT 7
#define BLUR_DETECT 8
#define FACE_DETECT 9
#define FACE_DETECT_RECOG 10
#define DUST_REMOVAL 11

#define WM_UPDATE_MESSAGE (WM_USER+200)
#define NTP_PORT 123
#define NTP_SERVER_IP "ntp1.aliyun.com"  
#define NTP_PORT_STR "123"        
#define JAN_1970 0x83aa7e80

constexpr DWORD max_time_sub = 3784147200;//12-31:3786825600;

struct NTPPacket
{
	union
	{
		struct _ControlWord
		{
			unsigned int uLI : 2;
			unsigned int uVersion : 3;
			unsigned int uMode : 3;
			unsigned int uStratum : 8;
			// 2 for next level, etc.
			int nPoll : 8;
			int nPrecision : 8;
		};

		int nControlWord;
	};

	int nRootDelay;
	int nRootDispersion;
	int nReferenceIdentifier;

	__int64 n64ReferenceTimestamp;
	__int64 n64OriginateTimestamp;
	__int64 n64ReceiveTimestamp;

	int nTransmitTimestampSeconds;
	int nTransmitTimestampFractions;
};

CEvent start_event;
int terminate_flag;

//全局变量
static float blur_thres = 4.0;
static bool g_bPlay = false;
static bool g_bPause = false;
static bool g_bSelectROI = false;
static int g_iDectType = -1;
double scale;
Mat g_matSourceFrame;
int real_width;
int real_height;
static vector<vector<Point>>g_vecROIlPoints;
static Mat g_matROIMask;
static UINT32 g_iTotalFrameNum;
static UINT32 g_iCurFrameIdx;
//检测器
static My_Detector g_dectDector;
//线程锁和缓冲队列
static CRITICAL_SECTION g_critSourceFrame;
static queue<Mat> g_queueSourceFrame;
static CRITICAL_SECTION g_critResultFrame;
static queue<Mat> g_queueResultFrame;
static CRITICAL_SECTION g_critPlayer;
// MyDlg 对话框

IMPLEMENT_DYNAMIC(MyDlg, CDialogEx)

MyDlg::MyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_detect_type(_T(""))
{
	//picture 背景
	InitialPic();
}

MyDlg::~MyDlg()
{
}

void MyDlg::ControlAllBtn(bool enable) 
{
	btn_open.EnableWindow(enable);
	btn_begin.EnableWindow(enable);
	btn_pause.EnableWindow(enable);
	btn_terminate.EnableWindow(enable);
	btn_insect_detect.EnableWindow(enable);
	btn_blur_detect.EnableWindow(enable);
	btn_dust_removal.EnableWindow(enable);
	btn_hat_detect.EnableWindow(enable);
	btn_smoke_detect.EnableWindow(enable);
	GetDlgItem(IDC_VIDEO_SLIDER)->EnableWindow(enable);
}

void MyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIDEO_SLIDER, m_video_slider);
	DDX_Text(pDX, IDC_DECT_EDIT, m_detect_type);
	DDX_Control(pDX, IDC_OPEN_BUTTON, btn_open);
	DDX_Control(pDX, IDC_BEGIN_BUTTON, btn_begin);
	DDX_Control(pDX, IDC_PAUSE_BUTTON, btn_pause);
	DDX_Control(pDX, IDC_STOP_BUTTON, btn_terminate);
	DDX_Control(pDX, IDC_INSECT_BUTTON, btn_insect_detect);
	DDX_Control(pDX, IDC_BULR_BUTTON, btn_blur_detect);
	DDX_Control(pDX, IDC_DEDUST_BUTTON, btn_dust_removal);
	DDX_Control(pDX, IDC_VIDEO_TIME_STATIC, m_video_time_static);
	DDX_Control(pDX, IDC_OPENURL_BUTTON, btn_url_open);
	DDX_Control(pDX, IDC_HELMAT_DETECT, btn_hat_detect);
	DDX_Control(pDX, IDC_SMOKE_DETECT, btn_smoke_detect);
}


BEGIN_MESSAGE_MAP(MyDlg, CDialogEx)
	ON_BN_CLICKED(IDC_OPEN_BUTTON, &MyDlg::OnBnClickedOpenButton)
	ON_BN_CLICKED(IDC_BEGIN_BUTTON, &MyDlg::OnBnClickedBeginButton)
	ON_BN_CLICKED(IDC_INSECT_BUTTON, &MyDlg::OnBnClickedInsectButton)
	ON_MESSAGE(WM_UPDATE_MESSAGE, &MyDlg::ResultMessage)
	ON_BN_CLICKED(IDC_BULR_BUTTON, &MyDlg::OnBnClickedBulrButton)
	ON_BN_CLICKED(IDC_DEDUST_BUTTON, &MyDlg::OnBnClickedDedustButton)
//	ON_WM_CREATE()
ON_WM_ACTIVATE()
ON_WM_SHOWWINDOW()
ON_WM_HSCROLL()
ON_WM_PAINT()
ON_BN_CLICKED(IDC_PAUSE_BUTTON, &MyDlg::OnBnClickedPauseButton)
ON_BN_CLICKED(IDC_STOP_BUTTON, &MyDlg::OnBnClickedStopButton)
ON_BN_CLICKED(IDC_HELMAT_DETECT, &MyDlg::OnBnClickedHelmatDetect)
ON_BN_CLICKED(IDC_SMOKE_DETECT, &MyDlg::OnBnClickedSmokeDetect)
ON_BN_CLICKED(IDC_OPENURL_BUTTON, &MyDlg::OnBnClickedOpenurlButton)
ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

//成员函数实现

/************************************************************************/
/* 从时间同步服务器获取时间信息                                         */
/************************************************************************/
DWORD MyDlg::GetTimeFromServer(char* ip_addr)
{
	// 参数ip_addr:表示指定的时间服务器IP地址
	// 返回：自1900年1月1日午0时0分0秒至今的毫秒数 或 0（表示获取失败）

	// 默认的时间服务器为"国家授时中心"
	if (ip_addr == nullptr)
	{
		ip_addr = "193.182.111.12";
	}

	// 定义WSADATA结构体对象
	WSADATA date;

	// 定义版本号码
	WORD w = MAKEWORD(2, 0);

	// 初始化套接字库
	if (::WSAStartup(w, &date) != 0)
	{
		MessageBox(_T("初始化套接字库失败！"));
		return 0;
	}

	// 定义连接套接字句柄
	SOCKET s;

	// 定义接收信息保存变量
	DWORD  m_serverTime;

	// 创建TCP套接字
	s = ::socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == s)
	{
		MessageBox(_T("创建套接字失败！"));

		// 关闭套接字句柄
		::closesocket(s);
		// 释放套接字库
		::WSACleanup();

		return 0;
	}

	// 定义套接字地址结构
	sockaddr_in addr;

	// 初始化地址结构
	addr.sin_family = AF_INET;
	addr.sin_port = htons(37);
	inet_pton(addr.sin_family, ip_addr, (void*)& addr.sin_addr.S_un.S_addr);
	
	// 连接
	if (::connect(s, (sockaddr*)& addr, sizeof(addr)) != 0)
	{
		int errorCode = ::WSAGetLastError();
		switch (errorCode)
		{
		case 10060:
			MessageBox(_T("连接超时！"));
			break;
		case 10051:
			MessageBox(_T("网络不可抵达!"));
			break;
		default:
			char temp[80];
			sprintf_s(temp, "WSAGetLastError()错误代码:%d", errorCode);
			CString temp_s(temp);
			MessageBox(temp_s);
		}

		// 关闭套接字句柄
		::closesocket(s);
		// 释放套接字库
		::WSACleanup();

		return 0;
	}

	// 接收
	if (::recv(s, (char*)& m_serverTime, 4, MSG_PEEK) <= 0)
	{
		MessageBox(_T("接收错误！"));

		// 关闭套接字句柄
		::closesocket(s);
		// 释放套接字库
		::WSACleanup();

		return 0;
	}

	// 关闭套接字句柄
	::closesocket(s);

	// 释放套接字库
	::WSACleanup();

	// 网络字节顺序转换为主机字节顺序
	m_serverTime = ::ntohl(m_serverTime);

	// 返回接收到的数据
	return m_serverTime;
}

// MyDlg 消息处理程序

void MyDlg::OnBnClickedOpenButton()
{
	CString filter = _T("video files(*.mp4; *.avi; *.h264; *.dav)|*.mp4; *.avi; *.h264; *.dav|ALL Files (*.*) |*.*||");
	CFileDialog dlg(TRUE/*这个参数为TRUE就是“打开”对话框，为FALSE就是“保存”对话框*/, NULL/*默认文件类型*/, NULL/*默认文件名*/, OFN_HIDEREADONLY/*样式，这里设置为“隐藏只读”*/, filter/*文件类型列表*/, NULL, NULL, FALSE/*指定文件打开对话框是否为Vista样式*/);
	if (dlg.DoModal() == IDOK)
	{
		TheImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
		strFilePath = dlg.GetPathName();

		current_pos = 0;
		USES_CONVERSION;
		const char* videoPath = T2A(strFilePath);
		vCap = new VideoCapture(videoPath);
		if (!vCap->isOpened())
		{
			return;
		}
		fps = (int)vCap->get(CV_CAP_PROP_FPS);
		frames = (int)vCap->get(CV_CAP_PROP_FRAME_COUNT);

		if (frames < 0)
		{
			//处理文件头没有帧数的情况，但还是不能实现进度条。
			//frames = GetAbnormalFrames(pThis->vCap);
		}

		if (frames != 0)
		{
			m_video_slider.SetRange(1, frames);
			m_video_slider.SetTicFreq(10);//设置显示刻度的间隔
			CTimeSpan cts((__time64_t)frames / fps);
			video_times.Format(_T("%d:%d:%d"), cts.GetHours(), cts.GetMinutes(), cts.GetSeconds());
		}
		Mat pMat;
		(*vCap) >> pMat;
		IplImage* pFrame = &IplImage(pMat);
		real_width = pFrame->width;
		real_height = pFrame->height;
		g_matSourceFrame = pMat;
		ResizeImage(pFrame);
		ShowImage(TheImage, IDC_VIDEOSHOW);
		//recover the program path
		//SetCurrentDirectory(path);//ocx（ActiveX）无法使用
		ButtomControl(true, true, false, false);

		InitializeCriticalSection(&g_critPlayer);
		InitializeCriticalSection(&g_critSourceFrame);
	}
}


void MyDlg::InitialPic()
{
	//picture 背景
	TheImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
	StopImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
	CannyImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 1);

}

void MyDlg::ShowImage(IplImage* image, UINT ID)
{
	CDC* pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	int rw = rect.right - rect.left;
	int rh = rect.bottom - rect.top;
	//	std::cout << rw << "  " << rh << std::endl; 
		/*CString str;
		str.Format(_T("%d\n%d"), rw , rh );
		MessageBox(str);*/

	int iw = image->width;
	int ih = image->height;
	/*CString str1;
	str1.Format(_T("%d\n%d"), iw, ih);
	MessageBox(str1);*/

	int tx = (int)(rw - iw) / 2;
	int ty = (int)(rh - ih) / 2;
	SetRect(rect, tx, ty, tx + iw, ty + ih);

	CvvImage cimg;
	cimg.CopyOf(image);
	cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);
}


int para[4] = { 0, 0, 0, 0 };

void Common_Resize(IplImage* img)
{
	float w = (float)img->width;
	float h = (float)img->height;

	float scalew = w / IMAGE_WIDTH;
	float scaleh = h / IMAGE_HEIGHT;
	scale = max(scalew, scaleh);
	int nw = (int)(w / scale), nh = (int)(h / scale);
	int tlx = (nw == IMAGE_WIDTH) ? 0 : (int)(IMAGE_WIDTH - nw) / 2;
	int tly = (nh == IMAGE_HEIGHT) ? 0 : (int)(IMAGE_HEIGHT - nh) / 2;
	para[0] = tlx; para[1] = tly; para[2] = nw; para[3] = nh;
}

void MyDlg::ResizeImage(IplImage* img)
{
	Common_Resize(img);
	try
	{
		cvSetImageROI(TheImage, cvRect(*para, *(para + 1), *(para + 2), *(para + 3)));
		cvResize(img, TheImage);
		cvResetImageROI(TheImage);
	}
	catch (Exception e) {}

}

void MyDlg::ResizeImage(IplImage* img, IplImage* to_img)
{
	Common_Resize(img);
	try
	{
		cvSetImageROI(to_img, cvRect(*para, *(para + 1), *(para + 2), *(para + 3)));
		cvResize(img, to_img);
		cvResetImageROI(to_img);
	}
	catch (Exception e) {}

}

afx_msg LRESULT MyDlg::ResultMessage(WPARAM wParam, LPARAM lParam)
{
	Mat result;
	switch (wParam)
	{
	case 0:
		EnterCriticalSection(&g_critResultFrame);
		if (!g_queueResultFrame.empty()) {
			g_queueResultFrame.front().copyTo(result);
			ResizeImage(&(IplImage)result, StopImage);
			ShowImage(StopImage, IDC_STATIC_IMGSHOW);
		}
		LeaveCriticalSection(&g_critResultFrame);
		break;
	case 1:
		UpdateData(FALSE);
		break;
	default:
		break;
	}
	return 0;
}

UINT ThreadDect(LPVOID pParm) {
	MyDlg* pThis = (MyDlg*)pParm;
	InitializeCriticalSection(&g_critResultFrame);
	//int queue_size;
	Mat dect_frame, res_frame, mask;
	ImageUtils iu(&dect_frame);
	while (g_bPlay) {
		if (g_bPause) {
			continue;
			Sleep(10);
		}
		Sleep(1);
		EnterCriticalSection(&g_critSourceFrame);
		if (g_queueSourceFrame.size()) {//取出队首帧进行处理
			g_queueSourceFrame.front().copyTo(dect_frame);
		}
		LeaveCriticalSection(&g_critSourceFrame);

		if (dect_frame.empty()) {
			continue;
		}

		if (g_bSelectROI && g_vecROIlPoints.size()) {
			bitwise_and(dect_frame, g_matROIMask, dect_frame);
		}
		//resize(dect_frame, dect_frame, Size(704, 576));
		iu.set_src(dect_frame);

		Sleep(10);

		switch (g_iDectType) {
		case TEXTURE://纹理检测
			//res_frame = g_dectDector.GetCannyImg(dect_frame);
			Sleep(10);
			break;
		case ANGPOINT://角点检测
			//res_frame = g_dectDector.GetCornerImg(dect_frame);
			Sleep(10);
			break;
		case HELMAT://安全帽检测
			res_frame = g_dectDector.GetHelmetImg(dect_frame);
			Sleep(10);
			break;
		case NIGHTENHANCE:
			//MSRCR增强
			res_frame = iu.main_msrcr_ex();
			//直方图匹配
			//res_frame = iu.get_hist_match();
			//去雾
			//res_frame = iu.deHaze_chai();
			Sleep(10);
			break;
		case DUST_REMOVAL:
			res_frame = iu.fast_deHaze();
			iu.set_src(res_frame);
			res_frame = iu.get_hist_match();
			Sleep(10);
			break;
		case SMOKING:
			res_frame = g_dectDector.GetSmokeImg(dect_frame);
			Sleep(10);
			break;
		case INSECT_DETECT:
			res_frame = iu.insect_detect();
			Sleep(10);
			break;
		case BLUR_DETECT:
			res_frame = iu.video_blur_detect(blur_thres);
			Sleep(10);
			break;
		case FACE_DETECT:
			//res_frame = iu.face_detect();
			Sleep(10);
			break;
		default:
			res_frame = dect_frame;
		}
		EnterCriticalSection(&g_critResultFrame);
		while (!g_queueResultFrame.empty()) {
			g_queueResultFrame.pop();
		}
		g_queueResultFrame.push(res_frame);
		LeaveCriticalSection(&g_critResultFrame);
		Sleep(10);
		pThis->PostMessage(WM_UPDATE_MESSAGE, 0, 0);//显示结果画面
		Sleep(30);
	}
	return 0;
}

//读取视频帧线程
DWORD WINAPI PlayVideo(LPVOID lpParam) {

	MyDlg* pThis = (MyDlg*)lpParam;//指针指向对话框

	Mat pMat;
	(*pThis->vCap) >> pMat;
	IplImage* pFrame = &IplImage(pMat);
	CString video_ctimes;
	while (!pMat.empty())
	{
		//设置slider位置
		++pThis->current_pos;
		pThis->m_video_slider.SetPos(pThis->current_pos);
		//更新视频时间
		if (pThis->current_pos % (pThis->fps) == 0)
		{
			CTimeSpan ccts((__time64_t)pThis->current_pos / pThis->fps);
			video_ctimes.Format(_T("%d:%d:%d /"), ccts.GetHours(), ccts.GetMinutes(), ccts.GetSeconds());
			pThis->m_video_time_static.SetWindowTextW(video_ctimes + pThis->video_times);
		}
		if (pThis->current_pos > pThis->frames)
		{
			pThis->current_pos = pThis->frames;
		}
		WaitForSingleObject(start_event, INFINITE);
		start_event.SetEvent();
		if (terminate_flag == -1)
		{
			terminate_flag = 0;
			_endthreadex(0);//关闭创建的线程
			break;
		};
		pThis->RealImage = pFrame;
		pThis->ResizeImage(pFrame);
		pThis->ShowImage(pThis->TheImage, IDC_VIDEOSHOW);//输出模板
		Sleep(1000 / pThis->fps);//间隔33毫秒
		if (!pThis->scrolling)
		{
			try
			{
				(*pThis->vCap) >> pMat;
			}
			catch (Exception e) {}
			if (g_bSelectROI) {//画出ROI区域
				for (int i = 0; i < g_vecROIlPoints.size(); i++) {
					drawContours(pMat, g_vecROIlPoints, i, Scalar(0, 0, 255), 3);
					Sleep(2);
				}
			}

			EnterCriticalSection(&g_critSourceFrame);
			while (!g_queueSourceFrame.empty()) {//每次队列中只保存最新的一帧，如果想保存多帧视频，在这里改动
				g_queueSourceFrame.pop();
			}
			g_queueSourceFrame.push(pMat);
			LeaveCriticalSection(&g_critSourceFrame);

			*pFrame = IplImage(pMat);
		}
	}
	if (terminate_flag != -1)
	{
		pThis->CloseVideo();
		pMat.release();
		pThis->MessageBox(_T("播放完毕!"), _T("播放"));
	}

	return 0;
}

void MyDlg::OnBnClickedBeginButton()
{
	g_bPlay = true;
	//开始播放视频代码
	SetTimer(1, 10, NULL);
	HANDLE hThreadSend;         //创建独立线程发送数据  
	DWORD ThreadSendID;

	start_event.SetEvent();

	hThreadSend = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayVideo, (LPVOID)this, 0, &ThreadSendID);
	ButtomControl(false, false, true, true);
	DetectButtomControl(true);

	//挂起检测线程
	m_threadVideoDect = AfxBeginThread(ThreadDect, this, 0, 0, CREATE_SUSPENDED, NULL);
	if (hThreadSend != (void*)0)
	{
		CloseHandle(hThreadSend);
	}
}


void MyDlg::OnBnClickedInsectButton()
{
	m_threadVideoDect->ResumeThread();
	g_iDectType = INSECT_DETECT;
	m_detect_type = "虫子检测";
	UpdateData(FALSE);
}


void MyDlg::OnBnClickedBulrButton()
{
	thresdlg.thres_index = static_cast<int>(blur_thres*2 - 2);
	if (thresdlg.DoModal() == IDOK)
	{
		blur_thres = thresdlg.threshold;
		m_threadVideoDect->ResumeThread();
		g_iDectType = BLUR_DETECT;
		m_detect_type = "套袋检测";
		UpdateData(FALSE);
	}
}


void MyDlg::OnBnClickedDedustButton()
{
	m_threadVideoDect->ResumeThread();
	g_iDectType = DUST_REMOVAL;
	m_detect_type = "灰尘去除";
	UpdateData(FALSE);
}


//int MyDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
//		return -1;
//
//	return 0;
//}


void MyDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);
}

string GetProgramDir()
{
	char exeFullPath[MAX_PATH]; // Full path   
	string strPath = "";
	wchar_t wtext[520];
	size_t converted = 0;
	mbstowcs_s(&converted, wtext, strlen(exeFullPath) + 1, exeFullPath, _TRUNCATE);//Plus null
	LPWSTR ptr = wtext;
	GetModuleFileName(nullptr, ptr, MAX_PATH);
	USES_CONVERSION;
	strPath = W2A(ptr);
	size_t pos = strPath.find_last_of('\\', strPath.length());
	return strPath.substr(0, pos);

}

void MyDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	ControlAllBtn(false);
	//控制时间
	auto now = GetTimeFromServer(nullptr);
	//now = max_time_sub + 1;
	if (now < max_time_sub)
		ButtomControl(true, false, false, false);
	auto resu = GetProgramDir();
	g_dectDector.SetPath(resu);
}

void MyDlg::OnVideoChange(int n)
{
	current_pos = n;
	vCap->set(CV_CAP_PROP_POS_MSEC, n * 1000 / fps);
}

void MyDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	scrolling = true;
	UpdateData(true);
	int m_nCur = m_video_slider.GetPos();//获取进度条当前位置
	UpdateData(false);
	OnVideoChange(m_nCur);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
	scrolling = false;
}


void MyDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
}


void MyDlg::OnBnClickedPauseButton()
{
	// TODO: 暂停视频代码
	CString buttonText;
	btn_pause.GetWindowText(buttonText);

	if (buttonText.Compare(_T("暂停")) == 0)
	{
		g_bPause = true;
		start_event.ResetEvent();
		btn_pause.SetWindowTextW(_T("继续"));
		try
		{
			//cvCopy(TheImage, StopImage);
			//ShowImage(StopImage, IDC_IMAGESHOW);
		}
		catch (Exception e) {}
	}
	else
	{
		g_bPause = false;
		//if (g_iDectType != -1)
		//{
		//	m_threadVideoDect->ResumeThread();
		//}
		start_event.SetEvent();
		btn_pause.SetWindowText(_T("暂停"));
	}
}


void MyDlg::OnBnClickedStopButton()
{
	// TODO: 结束播放视频代码
	terminate_flag = -1;
	CloseVideo();
	btn_pause.SetWindowText(_T("暂停"));
}

void MyDlg::CloseVideo()
{
	g_bPlay = false;
	try
	{
		//TerminateThread(m_threadVideoCap->m_hThread, 0);
		Sleep(10);
		m_threadVideoDect->SuspendThread();
		Sleep(10);
		m_detect_type = "当前无检测";
	}
	catch (Exception e) {}

	if (g_bSelectROI) {
		g_bSelectROI = false;
		//SetDlgItemText(IDC_ROI_BUTTON, _T("显示 ROI"));
	}

	cvReleaseData(TheImage);
	cvReleaseData(StopImage);

	InitialPic();
	ShowImage(TheImage, IDC_VIDEOSHOW);
	ShowImage(StopImage, IDC_STATIC_IMGSHOW);

	ButtomControl(true, false, false, false);
	DetectButtomControl(false);
	//cvReleaseCapture(&pCapture);//释放CvCapture结构

	g_dectDector.video_terminate();
	//Sleep(200);
	//vCap->release();
}

void MyDlg::ButtomControl(bool open, bool start, bool stop, bool terminate)
{
	btn_open.EnableWindow(open);
	btn_url_open.EnableWindow(open);
	btn_begin.EnableWindow(start);
	btn_pause.EnableWindow(stop);
	btn_terminate.EnableWindow(terminate);
}

void MyDlg::DetectButtomControl(bool enable)
{
	btn_insect_detect.EnableWindow(enable);
	btn_dust_removal.EnableWindow(enable);
	btn_blur_detect.EnableWindow(enable);
	btn_hat_detect.EnableWindow(enable);
	btn_smoke_detect.EnableWindow(enable);
	GetDlgItem(IDC_VIDEO_SLIDER)->EnableWindow(enable);
}


void MyDlg::OnBnClickedHelmatDetect()
{
	m_threadVideoDect->ResumeThread();
	g_iDectType = HELMAT;
	m_detect_type = "安全帽检测";
	UpdateData(FALSE);
}


void MyDlg::OnBnClickedSmokeDetect()
{
	m_threadVideoDect->ResumeThread();
	g_iDectType = SMOKING;
	m_detect_type = "抽烟检测";
	UpdateData(FALSE);
}


void MyDlg::OnBnClickedOpenurlButton()
{
	//输入视频所在URL
	if (urlin.DoModal() == IDOK)
	{
		TheImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
		strFilePath = urlin.getInputUrl();

		current_pos = 0;
		USES_CONVERSION;
		const char* videoPath = T2A(strFilePath);
		vCap = new VideoCapture(videoPath);
		if (!vCap->isOpened())
		{
			return;
		}
		fps = (int)vCap->get(CV_CAP_PROP_FPS);
		frames = (int)vCap->get(CV_CAP_PROP_FRAME_COUNT);

		if (frames < 0)
		{
			//处理文件头没有帧数的情况，但还是不能实现进度条。
			//frames = GetAbnormalFrames(pThis->vCap);
		}

		if (frames > 0)
		{
			m_video_slider.SetRange(1, frames);
			m_video_slider.SetTicFreq(10);//设置显示刻度的间隔
			CTimeSpan cts((__time64_t)frames / fps);
			video_times.Format(_T("%d:%d:%d"), cts.GetHours(), cts.GetMinutes(), cts.GetSeconds());
		}
		Mat pMat;
		(*vCap) >> pMat;
		IplImage* pFrame = &IplImage(pMat);
		real_width = pFrame->width;
		real_height = pFrame->height;
		g_matSourceFrame = pMat;
		ResizeImage(pFrame);
		ShowImage(TheImage, IDC_VIDEOSHOW);
		//recover the program path
		//SetCurrentDirectory(path);
		ButtomControl(true, true, false, false);

		DetectButtomControl(false);
		InitializeCriticalSection(&g_critPlayer);
		InitializeCriticalSection(&g_critSourceFrame);
	}
}

void MyDlg::OpenByJSWholeURL(CString url)
{
	TheImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
	strFilePath = url;

	current_pos = 0;
	USES_CONVERSION;
	const char* videoPath = T2A(strFilePath);
	vCap = new VideoCapture(videoPath);
	if (!vCap->isOpened())
	{
		return;
	}
	fps = (int)vCap->get(CV_CAP_PROP_FPS);
	frames = (int)vCap->get(CV_CAP_PROP_FRAME_COUNT);

	if (frames < 0)
	{
		//处理文件头没有帧数的情况，但还是不能实现进度条。
		//frames = GetAbnormalFrames(pThis->vCap);
	}

	if (frames > 0)
	{
		m_video_slider.SetRange(1, frames);
		m_video_slider.SetTicFreq(10);//设置显示刻度的间隔
		CTimeSpan cts((__time64_t)frames / fps);
		video_times.Format(_T("%d:%d:%d"), cts.GetHours(), cts.GetMinutes(), cts.GetSeconds());
	}
	Mat pMat;
	(*vCap) >> pMat;
	IplImage* pFrame = &IplImage(pMat);
	real_width = pFrame->width;
	real_height = pFrame->height;
	g_matSourceFrame = pMat;
	ResizeImage(pFrame);
	ShowImage(TheImage, IDC_VIDEOSHOW);
	//recover the program path
	//SetCurrentDirectory(path);
	ButtomControl(true, true, false, false);

	DetectButtomControl(false);
	InitializeCriticalSection(&g_critPlayer);
	InitializeCriticalSection(&g_critSourceFrame);
}
void MyDlg::OpenByJSNecessaryText(int brand, int protocol, CString url_ip, int url_port, CString username, CString password, int channel, int subtype, int old_version, int codec)
{
	//MessageBox(NULL, TEXT("坏蛋可以去死啦!"), 0);
	TheImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);

	CString final_url, port, streamtype, channel_c;
	port.Format(_T("%d"), url_port);
	streamtype.Format(_T("%d"), subtype);
	channel_c.Format(_T("%d"), channel);
	//0360 1大华 2海康 3其他
	//大华格式：rtsp://username:password@ip:port/cam/realmonitor?channel=1&subtype=0
	if (protocol == 1)
	{
		final_url = _T("rtmp://");
	}
	else if (protocol == 2)
	{
		final_url = _T("rtsp://");
	}
	else
	{
		final_url = _T("http://");
	}
	switch (brand)
	{
	case 1:
		final_url += username + ':' + password + '@' + url_ip + ':' + port + _T("/cam/realmonitor?channel=") + channel_c + _T("&subtype=") + streamtype;
		break;
	case 2:
		final_url += username + ':' + password + '@' + url_ip + ':' + port;
		if (old_version == 0)
		{
			if (codec == 0)
			{
				final_url += _T("/h264/");
			}
			else
			{
				final_url += _T("/mpeg4/");
			}
			final_url += _T("ch") + channel_c;
			if (subtype == 0)
			{
				final_url += _T("/main/");
			}
			else if (subtype == 1)
			{
				final_url += _T("/sub/");
			}
			else
			{
				final_url += _T("/stream") + streamtype + '/';
			}
			final_url += _T("av_stream");
		}
		else
		{
			CString sb;
			if (subtype == 0 || subtype == 1)
				sb.Format(_T("%d"), subtype + 1);
			else
				sb = streamtype;
			final_url += _T("/Streaming/Channels/") + channel_c + '0' + sb + _T("?transportmode=unicast");
		}

		break;
	default:
		break;
	}
	//MessageBox(NULL, final_url, 0);

	strFilePath = final_url;

	current_pos = 0;
	USES_CONVERSION;
	const char* videoPath = T2A(strFilePath);
	try {
		vCap = new VideoCapture(videoPath);
	}
	catch (Exception e) { return; }
	if (!vCap->isOpened())
	{
		return;
	}
	fps = (int)vCap->get(CV_CAP_PROP_FPS);
	frames = (int)vCap->get(CV_CAP_PROP_FRAME_COUNT);

	if (frames < 0)
	{
		//处理文件头没有帧数的情况，但还是不能实现进度条。
		//frames = GetAbnormalFrames(pThis->vCap);
	}

	if (frames > 0)
	{
		m_video_slider.SetRange(1, frames);
		m_video_slider.SetTicFreq(10);//设置显示刻度的间隔
		CTimeSpan cts((__time64_t)frames / fps);
		video_times.Format(_T("%d:%d:%d"), cts.GetHours(), cts.GetMinutes(), cts.GetSeconds());
	}
	Mat pMat;
	(*vCap) >> pMat;
	IplImage* pFrame = &IplImage(pMat);
	real_width = pFrame->width;
	real_height = pFrame->height;
	g_matSourceFrame = pMat;
	ResizeImage(pFrame);
	ShowImage(TheImage, IDC_VIDEOSHOW);
	//recover the program path
	//SetCurrentDirectory(path);
	ButtomControl(true, true, false, false);

	DetectButtomControl(false);
	InitializeCriticalSection(&g_critPlayer);
	InitializeCriticalSection(&g_critSourceFrame);
}


BOOL MyDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}
