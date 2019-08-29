
// VideoDemoDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "VideoDemo.h"
#include "VideoDemoDlg.h"
#include "VideoPlay.h"
#include "afxdialogex.h"
#include "CvvImage.h"
#include "DealWithTensorFlow.h"
#include "PretrainDetector.h"
#include "ImageUtils.h"

using namespace cv;
using namespace std;

CEvent start_event;
int terminate_flag;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

//全局变量
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

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVideoDemoDlg 对话框

CVideoDemoDlg::CVideoDemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VIDEODEMO_DIALOG, pParent)
	, m_detect_type(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_LOGO_ICON);
	strFilePath = _T("");
	strFileName = _T("");
}

void CVideoDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DETECT_TYPE_EDIT, m_detect_type);
	DDX_Control(pDX, IDC_EDIT1, m_file);
	DDX_Control(pDX, IDC_VIDEOSHOW, m_video);
	DDX_Control(pDX, IDC_IMAGESHOW, m_image);
	DDX_Control(pDX, IDC_VIDEO_STOP_BUTTON, StopButton);
	DDX_Control(pDX, IDC_VIDEO_START_BUTTON, StartButton);
	DDX_Control(pDX, IDC_VIDEO_TERMINATE_BUTTON, TerminateButton);
	//  DDX_Control(pDX, IDC_VIDEO_SLIDER, m_video_slider);
	DDX_Control(pDX, IDC_VIDEO_SLIDER, m_video_slider);
	DDX_Control(pDX, IDC_VIDEO_TIME_STATIC, m_video_time_static);
	DDX_Control(pDX, IDC_OPEN_BUTTON, OpenButton);

	DDX_Control(pDX, IDC_SNAT_BUTTON, SnatButton);
	DDX_Control(pDX, IDC_NIGHT_ENHANCE_BUTTON, NightEnhanceButton);
	DDX_Control(pDX, IDC_TEXTURE_BUTTON, TextureButton);
	DDX_Control(pDX, IDC_CORNER_BUTTON, CornerButton);
	DDX_Control(pDX, IDC_HUMAN_TRACK_BUTTON, HumanTraceButton);
	DDX_Control(pDX, IDC_SAFEHAT_DETECT_BUTTON, HelmatButton);
	DDX_Control(pDX, IDC_SMOKING_DETECT_BUTTON, SmokingButton);
	DDX_Control(pDX, IDC_SET_ROI_BUTTON, ROISetButton);
	DDX_Control(pDX, IDC_SHOW_ROI_BUTTON, ShowROIButton);
}

BEGIN_MESSAGE_MAP(CVideoDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_32773, &CVideoDemoDlg::On32773)
	ON_COMMAND(ID_32771, &CVideoDemoDlg::On32771)
	ON_BN_CLICKED(IDOK, &CVideoDemoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoDemoDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_NIGHT_ENHANCE_BUTTON, &CVideoDemoDlg::OnBnClickedNightEnhanceButton)
	ON_BN_CLICKED(IDC_VIDEO_START_BUTTON, &CVideoDemoDlg::OnBnClickedVideoStartButton)
	ON_BN_CLICKED(IDC_VIDEO_STOP_BUTTON, &CVideoDemoDlg::OnBnClickedVideoStopButton)
	ON_BN_CLICKED(IDC_VIDEO_TERMINATE_BUTTON, &CVideoDemoDlg::OnBnClickedVideoTerminateButton)
	ON_COMMAND(ID_32774, &CVideoDemoDlg::On32774)
	ON_BN_CLICKED(IDC_SAFEHAT_DETECT_BUTTON, &CVideoDemoDlg::OnBnClickedSafehatDetectButton)
	ON_BN_CLICKED(IDC_SMOKING_DETECT_BUTTON, &CVideoDemoDlg::OnBnClickedSmokingDetectButton)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIDEO_SLIDER, &CVideoDemoDlg::OnNMCustomdrawVideoSlider)
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_STN_CLICKED(IDC_VIDEO_TIME_STATIC, &CVideoDemoDlg::OnStnClickedVideoTimeStatic)
	ON_BN_CLICKED(IDC_TEST_BUTTON, &CVideoDemoDlg::OnBnClickedTestButton)
	ON_BN_CLICKED(IDC_OPEN_BUTTON, &CVideoDemoDlg::OnBnClickedOpenButton)
	ON_BN_CLICKED(IDC_SNAT_BUTTON, &CVideoDemoDlg::OnBnClickedSnatButton)
	ON_BN_CLICKED(IDC_SET_ROI_BUTTON, &CVideoDemoDlg::OnBnClickedSetRoiButton)
	ON_BN_CLICKED(IDC_SHOW_ROI_BUTTON, &CVideoDemoDlg::OnBnClickedShowRoiButton)
	ON_MESSAGE(WM_UPDATE_MESSAGE, &CVideoDemoDlg::ResultMessage)
	ON_MESSAGE(WM_UPDARA_CONFIG, &CVideoDemoDlg::UpdateROIData)
	//ON_MESSAGE(WM_HDHYY, &CVideoDemoDlg::OnHdhyy)
	ON_BN_CLICKED(IDC_TEXTURE_BUTTON, &CVideoDemoDlg::OnBnClickedTextureButton)
	ON_BN_CLICKED(IDC_CORNER_BUTTON, &CVideoDemoDlg::OnBnClickedCornerButton)
	ON_BN_CLICKED(IDC_INSECT_BUTTON, &CVideoDemoDlg::OnBnClickedInsectButton)
	ON_BN_CLICKED(IDC_BLUR_DETECT_BUTTON, &CVideoDemoDlg::OnBnClickedBlurDetectButton)
	ON_BN_CLICKED(IDC_FACE_DETECT_BUTTON, &CVideoDemoDlg::OnBnClickedFaceDetectButton)
	ON_BN_CLICKED(IDC_HUMAN_TRACK_BUTTON, &CVideoDemoDlg::OnBnClickedHumanTrackButton)
	ON_BN_CLICKED(IDC_DUST_BUTTON, &CVideoDemoDlg::OnBnClickedDustButton)
END_MESSAGE_MAP()


// CVideoDemoDlg 消息处理程序

BOOL CVideoDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	//CMenu menu;
	//menu.LoadMenuW(IDR_MENU1);
	//SetMenu(&menu);
	//初始化图像处理的代码
	InitialPic();

	//初始化路径
	path = new TCHAR[MAX_PATH];
	ZeroMemory(path, MAX_PATH);
	GetCurrentDirectory(MAX_PATH, path);
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	scale = 1.0;
	ButtomControl(true, false, false, false);
	m_detect_type = "当前无检测";
	UpdateData(false);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CVideoDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVideoDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		CDialog::UpdateWindow();
		//ShowImage(TheImage, IDC_VIDEOSHOW);
		//ShowImage(StopImage, IDC_IMAGESHOW);
	}
}

void CVideoDemoDlg::InitialPic()
{
	//picture 背景
	TheImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
	StopImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
	CannyImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 1);
	DisableAllDetectButton(false);
}

void CVideoDemoDlg::ShowImage(IplImage* image, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
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

void CVideoDemoDlg::ResizeImage(IplImage* img)
{
	Common_Resize(img);
	try
	{
		cvSetImageROI(TheImage, cvRect(*para, *(para+1), *(para + 2), *(para + 3)));
		cvResize(img, TheImage);
		cvResetImageROI(TheImage);
	}
	catch (Exception e) {}

}

void CVideoDemoDlg::ResizeImage(IplImage* img, IplImage* to_img)
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

void CVideoDemoDlg::ButtomControl(bool open, bool start, bool stop, bool terminate)
{
	OpenButton.EnableWindow(open);
	StartButton.EnableWindow(start);
	StopButton.EnableWindow(stop);
	TerminateButton.EnableWindow(terminate);
}

void CVideoDemoDlg::DisableAllDetectButton(bool b)
{
	SnatButton.EnableWindow(b);
	HelmatButton.EnableWindow(b);
	CornerButton.EnableWindow(b);
	TextureButton.EnableWindow(b);
	NightEnhanceButton.EnableWindow(b);
	HumanTraceButton.EnableWindow(b);
	SmokingButton.EnableWindow(b);
	ROISetButton.EnableWindow(b);
	ShowROIButton.EnableWindow(b);
}

void CVideoDemoDlg::OnVideoChange(int n)
{
	current_pos = n;
	vCap->set(CV_CAP_PROP_POS_MSEC, n * 1000 / fps);
}

int GetAbnormalFrames(VideoCapture *vcp)
{
	int fc = 0;
	Mat pMat;
	(*vcp) >> pMat;
	while (1)
	{
		(*vcp) >> pMat;
		if (pMat.empty())
		{
			break;
		}
		++fc;
	}
	vcp->set(CV_CAP_PROP_POS_MSEC, 0);
	return fc;
}

bool CVideoDemoDlg::ReadROIFile(CString cstrFileName) {
	if (!PathFileExists(cstrFileName)) {
		MessageBox(_T("未发现ROI配置文件!"), _T("ROI"));
		return false;
	}

	g_matSourceFrame.copyTo(g_matROIMask);
	g_matROIMask.setTo(Scalar::all(0));

	//读入ROI区域
	g_vecROIlPoints.clear();
	vector<Point> points_vec;
	points_vec.clear();
	CString strLine, strTemp, result;
	int row = 0;
	double width_zoom_factor, hight_zoom_factor;
	double x, y;

	CStdioFile file(cstrFileName, CFile::modeRead);
	while (file.ReadString(strLine))
	{
		if (row == 0) {
			width_zoom_factor = _tstof(strLine);
		}
		else if (row == 1) {
			hight_zoom_factor = _tstof(strLine);
		}//前两行读入缩放因子
		else {
			char *str = (char *)strLine.GetBufferSetLength(strLine.GetLength());
			if (strLine != "" && strLine != "--")
			{
				CStringArray m_strArray[5];
				TCHAR seps[] = _T(",");
				TCHAR *next_token = NULL;
				TCHAR* token = _tcstok_s((LPTSTR)(LPCTSTR)strLine, seps, &next_token);
				while (token != NULL)
				{
					m_strArray[0].Add(token);
					token = _tcstok_s(NULL, seps, &next_token);
				}
				int count = (int)m_strArray[0].GetSize();
				x = _tstof(m_strArray[0].GetAt(0)) * width_zoom_factor;
				y = _tstof(m_strArray[0].GetAt(1)) * hight_zoom_factor;
				points_vec.push_back(Point((int)x, (int)y));
			}
			else if (strLine == "--") {
				g_vecROIlPoints.push_back(points_vec);
				points_vec.clear();
			}
		}
		row++;
	}
	file.Close();
	//更新掩模
	fillPoly(g_matROIMask, g_vecROIlPoints, Scalar(255, 255, 255), 8, 0);
	//resize(g_matROIMask, g_matROIMask, Size(704, 576));
	return true;
}

UINT ThreadDect(LPVOID pParm) {
	CVideoDemoDlg* pThis = (CVideoDemoDlg*)pParm;
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
			res_frame = g_dectDector.GetCannyImg(dect_frame);
			Sleep(10);
			break;
		case ANGPOINT://角点检测
			res_frame = g_dectDector.GetCornerImg(dect_frame);
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
			res_frame = iu.brighten();
			//iu.set_src(res_frame);
			//res_frame = iu.get_hist_match();
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
			res_frame = iu.video_blur_detect();
			Sleep(10);
			break;
		case FACE_DETECT:
			res_frame = iu.face_detect();
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

	CVideoDemoDlg* pThis = (CVideoDemoDlg*)lpParam;//指针指向对话框

	Mat pMat;
	(*pThis->vCap) >> pMat;
	IplImage *pFrame = &IplImage(pMat);
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
		};
		pThis->RealImage = pFrame;
		pThis->ResizeImage(pFrame);
		pThis->ShowImage(pThis->TheImage, IDC_VIDEOSHOW);//输出模板
		Sleep(1000/ pThis->fps);//间隔33毫秒
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
	pThis->CloseVideo();
	pMat.release();
	pThis->MessageBox(_T("播放完毕!"), _T("播放"));
	return 0;
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVideoDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVideoDemoDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}

void CVideoDemoDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

void CVideoDemoDlg::On32773()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg cab;
	cab.DoModal();
}

void CVideoDemoDlg::On32771()
{
	//为按钮添加打开文件的功能，并默认打开.png格式的文件，并在打开文件右下角添加文件类型选项。
	OnBnClickedOpenButton();
}

void CVideoDemoDlg::On32774()
{
	// TODO: 打开图片的代码
	CFileDialog dlg(true, NULL, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, _T("image files(*.jpg; *.bmp)|*.jpg; *.bmp|ALL Files (*.*) |*.*||"), NULL);
	dlg.m_ofn.lpstrTitle = _T("Open Image");
	if (dlg.DoModal() == IDOK)
	{
		ATL::CImage image;
		image.Load(dlg.GetPathName());
		//将打开的图片显示在picture控件上。
		CRect rectControl;                   //控件矩形对象
		m_video.GetClientRect(rectControl);
		CDC *pDc = m_video.GetDC();			 //设备上下文对象的类
		rectControl = CRect(rectControl.TopLeft(), CSize((int)rectControl.Width(), (int)rectControl.Height()));
		m_video.SetBitmap(NULL);				//清空picture
		image.Draw(pDc->m_hDC, rectControl);    //将图片绘制到Picture控件表示的矩形区域
		image.Destroy();
		m_video.ReleaseDC(pDc);
	}
}


void CVideoDemoDlg::OnBnClickedNightEnhanceButton()
{
	// TODO:夜晚图像增强代码
	m_threadVideoDect->ResumeThread();
	g_iDectType = NIGHTENHANCE;
	m_detect_type = "暗图像增强";
	UpdateData(FALSE);
}

void CVideoDemoDlg::Canny()
{
	//Canny边缘检测算子
	IplImage *gray = 0, *edge = 0;
	gray = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 1);
	edge = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 1);
	cvCvtColor(StopImage, gray, CV_BGR2GRAY);
	cvCanny(gray, edge, 30, 100, 3);
	cvCvtColor(edge, StopImage, CV_GRAY2BGR);
	ShowImage(StopImage, IDC_IMAGESHOW);
	cvReleaseImage(&gray);
	cvReleaseImage(&edge);
}

void CVideoDemoDlg::OnBnClickedOpenButton()
{
	//选择视频所在的目录
	CString filter = _T("video files(*.mp4; *.avi; *.h264; *.dav)|*.mp4; *.avi; *.h264; *.dav|ALL Files (*.*) |*.*||");
	CFileDialog dlg(TRUE/*这个参数为TRUE就是“打开”对话框，为FALSE就是“保存”对话框*/, NULL/*默认文件类型*/, NULL/*默认文件名*/, OFN_HIDEREADONLY/*样式，这里设置为“隐藏只读”*/, filter/*文件类型列表*/, NULL, NULL, FALSE/*指定文件打开对话框是否为Vista样式*/);
	if (dlg.DoModal() == IDOK)
	{
		TheImage = cvCreateImage(CvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
		strFilePath = dlg.GetPathName();

		current_pos = 0;
		USES_CONVERSION;
		const char *videoPath = T2A(strFilePath);
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
		SetCurrentDirectory(path);
		ButtomControl(true, true, false, false);

		DisableAllDetectButton(true);
		InitializeCriticalSection(&g_critPlayer);
		InitializeCriticalSection(&g_critSourceFrame);
	}
}


void CVideoDemoDlg::OnBnClickedVideoStartButton()
{
	g_bPlay = true;
	//开始播放视频代码
	SetTimer(1, 10, NULL);
	HANDLE hThreadSend;         //创建独立线程发送数据  
	DWORD ThreadSendID;

	start_event.SetEvent();

	hThreadSend = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayVideo, (LPVOID)this, 0, &ThreadSendID);
	ButtomControl(false, false, true, true);

	//挂起检测线程
	m_threadVideoDect = AfxBeginThread(ThreadDect, this, 0, 0, CREATE_SUSPENDED, NULL);
	CloseHandle(hThreadSend);
}


void CVideoDemoDlg::OnBnClickedVideoStopButton()
{
	// TODO: 暂停视频代码
	CString buttonText;
	StopButton.GetWindowText(buttonText);

	if (buttonText.Compare(_T("暂停")) == 0)
	{
		g_bPause = true;
		start_event.ResetEvent();
		StopButton.SetWindowTextW(_T("继续"));
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
		StopButton.SetWindowText(_T("暂停"));
	}
}


void CVideoDemoDlg::OnBnClickedVideoTerminateButton()
{
	// TODO: 结束播放视频代码
	terminate_flag = -1;
	StopButton.SetWindowText(_T("暂停"));
	CloseVideo();
}


void CVideoDemoDlg::OnBnClickedSafehatDetectButton()
{
	//Mat r = g_dectDector.GetHelmetImg(cvarrToMat(RealImage));
	//imshow("result", r);
	m_threadVideoDect->ResumeThread();
	g_iDectType = HELMAT;
	m_detect_type = "安全帽检测";
	UpdateData(FALSE);
}


void CVideoDemoDlg::OnBnClickedSmokingDetectButton()
{
	// TODO: 抽烟检测的代码
	m_threadVideoDect->ResumeThread();
	g_iDectType = SMOKING;
	m_detect_type = "抽烟检测";
	UpdateData(FALSE);
}


void CVideoDemoDlg::OnNMCustomdrawVideoSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CVideoDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}


void CVideoDemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	scrolling = true;
	UpdateData(true);
	int m_nCur = m_video_slider.GetPos();//获取进度条当前位置
	UpdateData(false);
	OnVideoChange(m_nCur);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
	scrolling = false;
}


void CVideoDemoDlg::OnStnClickedVideoTimeStatic()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CVideoDemoDlg::OnBnClickedTestButton()
{
	// TODO: 在此添加控件通知处理程序代码
	VideoPlay vp;
	vp.DoModal();
}


void CVideoDemoDlg::OnBnClickedSnatButton()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CVideoDemoDlg::OnBnClickedSetRoiButton()
{
	// TODO:  在此添加控件通知处理程序代码
	m_threadGetROI = new ROIThread();
	m_threadGetROI->CreateThread();
	m_threadGetROI->m_hWnd = m_hWnd;
	//将画面传入标定子线程
	EnterCriticalSection(&g_critSourceFrame);
	if (!g_queueSourceFrame.empty()) {
		g_matSourceFrame = g_queueSourceFrame.front();
	}
	LeaveCriticalSection(&g_critSourceFrame);
}


void CVideoDemoDlg::OnBnClickedShowRoiButton()
{
	CString str;
	GetDlgItem(IDC_SHOW_ROI_BUTTON)->GetWindowText(str);
	if (str == _T("显示 ROI")) {
		if (!ReadROIFile(g_cstrROIFile)) {
			return;
		}
		g_bSelectROI = true;
		SetDlgItemText(IDC_SHOW_ROI_BUTTON, _T("显示原画面"));
	}
	else if (str == _T("显示原画面")) {
		g_bSelectROI = false;
		SetDlgItemText(IDC_SHOW_ROI_BUTTON, _T("显示 ROI"));
	}
}


afx_msg LRESULT CVideoDemoDlg::ResultMessage(WPARAM wParam, LPARAM lParam)
{
	Mat result;
	switch (wParam)
	{
	case 0:
		EnterCriticalSection(&g_critResultFrame);
		if (!g_queueResultFrame.empty()) {
			g_queueResultFrame.front().copyTo(result);
			ResizeImage(&(IplImage)result, StopImage);
			ShowImage(StopImage, IDC_IMAGESHOW);
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


afx_msg LRESULT CVideoDemoDlg::UpdateROIData(WPARAM wParam, LPARAM lParam)
{
	//圈定完 自动更新ROI区域
	ReadROIFile(g_cstrROIFile);
	return 0;
}


void CVideoDemoDlg::OnBnClickedTextureButton()
{
	// TODO: 在此添加控件通知处理程序代码
	m_threadVideoDect->ResumeThread();
	g_iDectType = TEXTURE;
	m_detect_type = "纹理检测";
	UpdateData(FALSE);
}

void CVideoDemoDlg::CloseVideo()
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
	catch (Exception e){}

	if (g_bSelectROI) {
		g_bSelectROI = false;
		SetDlgItemText(IDC_ROI_BUTTON, _T("显示 ROI"));
	}

	cvReleaseData(TheImage);
	cvReleaseData(StopImage);

	InitialPic();
	ShowImage(TheImage, IDC_VIDEOSHOW);
	ShowImage(StopImage, IDC_IMAGESHOW);

	ButtomControl(true, false, false, false);
	cvReleaseCapture(&pCapture);//释放CvCapture结构

	g_dectDector.video_terminate();
	Sleep(100);
	vCap->release();
}


void CVideoDemoDlg::OnBnClickedCornerButton()
{
	// TODO: 在此添加控件通知处理程序代码
	m_threadVideoDect->ResumeThread();
	g_iDectType = ANGPOINT;
	m_detect_type = "角点检测";
	UpdateData(FALSE);
}


void CVideoDemoDlg::OnBnClickedInsectButton()
{
	// TODO: 在此添加控件通知处理程序代码
	m_threadVideoDect->ResumeThread();
	g_iDectType = INSECT_DETECT;
	m_detect_type = "虫子检测";
	UpdateData(FALSE);
}


void CVideoDemoDlg::OnBnClickedBlurDetectButton()
{
	m_threadVideoDect->ResumeThread();
	g_iDectType = BLUR_DETECT;
	m_detect_type = "套袋检测";
	UpdateData(FALSE);
}


void CVideoDemoDlg::OnBnClickedFaceDetectButton()
{
	m_threadVideoDect->ResumeThread();
	g_iDectType = FACE_DETECT;
	m_detect_type = "人脸检测";
	UpdateData(FALSE);
}


void CVideoDemoDlg::OnBnClickedHumanTrackButton()
{
	
}


void CVideoDemoDlg::OnBnClickedDustButton()
{
	m_threadVideoDect->ResumeThread();
	g_iDectType = DUST_REMOVAL;
	m_detect_type = "去灰尘";
	UpdateData(FALSE);
}
