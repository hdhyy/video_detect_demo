// ROIDialog.cpp: 实现文件
//

#include "stdafx.h"
#include "VideoDemo.h"
#include "ROIDialog.h"
#include "afxdialogex.h"

//全局变量
static ROIDialog* g_cDlg = NULL;
static bool g_bDisplay = true;
static Mat g_matFrameROI;
CString g_cstrROIFile = _T("./roi.ini");

// ROIDialog 对话框

IMPLEMENT_DYNAMIC(ROIDialog, CDialogEx)

ROIDialog::ROIDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ROI_DIALOG, pParent)
{

}

ROIDialog::~ROIDialog()
{
}

void ROIDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ROIDialog, CDialogEx)
	ON_BN_CLICKED(IDC_SAVE_ROI_BUTTON, &ROIDialog::OnBnClickedSaveRoiButton)
	ON_BN_CLICKED(IDC_CLEAR_BUTTON, &ROIDialog::OnBnClickedClearButton)
END_MESSAGE_MAP()

static vector<vector<Point>>g_vecTotalPoints;
static vector<Point> g_vecPoints;
static Point g_ptStart = (-1, -1);
static Point g_ptCur = (-1, -1);
static bool g_bBegin = false;

void PiexLocation_Mouse(int EVENT, int x, int y, int flags, void* userdata) {
	ROIDialog* pThis = (ROIDialog*)g_cDlg;
	Mat img;
	img = *(Mat*)userdata;
	if (EVENT == EVENT_LBUTTONDOWN && !g_bBegin) {//首次按下左键，开始圈定
		g_ptStart = Point(x, y);
		g_vecPoints.push_back(g_ptStart);
		circle(img, g_ptStart, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		g_bBegin = true;
	}
	else if (EVENT == EVENT_LBUTTONDOWN && g_bBegin) {//再次按下左键
		g_ptCur = Point(x, y);
		line(img, g_vecPoints.back(), g_ptCur, Scalar(0, 255, 0, 0), 1, 8, 0);
		circle(img, g_ptCur, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		g_vecPoints.push_back(g_ptCur);
		if (abs(g_ptCur.x - g_ptStart.x) < 5 && abs(g_ptCur.y - g_ptStart.y) < 5) {
			line(img, g_ptStart, g_ptCur, Scalar(0, 255, 0, 0), 1, 8, 0);
			const Point * ppt[1] = { &g_vecPoints[0] };//取数组的首地址
			int len = g_vecPoints.size();
			int npt[] = { len };
			g_vecTotalPoints.push_back(g_vecPoints);//存入该组
			fillPoly(img, ppt, npt, 1, cv::Scalar(0, 255, 255, 255));
			//fillPoly(img, g_vecTotalPoints, Scalar(255, 255, 255), 8, 0);  //也可使用此方式进行填充
			g_bBegin = false;
			g_vecPoints.clear();
		}
	}
}

// ROIDialog 消息处理程序
UINT ThreadGetROI(LPVOID pParm) {
	setMouseCallback("src1", PiexLocation_Mouse, (void*)&g_matFrameROI);
	while (g_bDisplay) {
		imshow("src1", g_matFrameROI);
		waitKey(40);
	}
	destroyAllWindows();
	return 0;
}

void ROIDialog::OnBnClickedSaveRoiButton()
{
	LPCTSTR cfg_name = (LPCTSTR)g_cstrROIFile;
	CStdioFile cfg_file;
	CFileException fileException;
	g_bDisplay = false;
	CString str;
	UpdateData(true);

	if (cfg_file.Open(cfg_name, CFile::typeText | CFile::modeCreate | CFile::modeReadWrite), &fileException)
	{
		//前两行写入缩放因子
		CString str_zoom_factor;
		str_zoom_factor.Format(_T("%lf"), m_zoomFactor);
		cfg_file.WriteString(str_zoom_factor + '\n');
		str_zoom_factor.Format(_T("%lf"), m_zoomFactor);
		cfg_file.WriteString(str_zoom_factor + '\n');
		//后面写入点的坐标
		for (int i = 0; i < g_vecTotalPoints.size(); i++) {
			for (int j = 0; j < g_vecTotalPoints[i].size(); j++) {
				str.Format(_T("%d,%d"), g_vecTotalPoints[i][j].x, g_vecTotalPoints[i][j].y);
				cfg_file.WriteString(str + '\n');
			}
			cfg_file.WriteString(_T("--\n"));
		}
		MessageBox(_T("保存成功"), _T("ROI"));
	}
	else
	{
		TRACE("Can't open file %s,error=%u\n", cfg_name, fileException.m_cause);
	}
}

void ROIDialog::ResizeImage()
{
	//g_matSourceFrame.copyTo(g_matFrameROI);
	IplImage *img = cvCreateImage(Size(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS);
	int nw = (int)(real_width / scale), nh = (int)(real_height / scale);
	int tlx = (nw > nh) ? 0 : (int)(IMAGE_WIDTH - nw) / 2;
	int tly = (nw > nh) ? (int)(IMAGE_HEIGHT - nh) / 2 : 0;
	cvSetImageROI(img, Rect(tlx, tly, nw, nh));
	cvResize(&(IplImage)g_matSourceFrame, img);
	cvResetImageROI(img);
	//resize(g_matFrameROI, g_matFrameROI, Size(real_width / scale, real_height / scale));
	Mat img_m = cvarrToMat(img);
	img_m.copyTo(g_matFrameROI);
}

void ROIDialog::OnBnClickedClearButton()
{
	// TODO:  在此添加控件通知处理程序代码
	g_vecPoints.clear();
	g_vecTotalPoints.clear();
	ResizeImage();
	g_bBegin = false;
}

BOOL ROIDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	g_cDlg = this;
	g_bDisplay = true;
	int width = g_matSourceFrame.cols, height = g_matSourceFrame.rows;
	m_dWidthZoomFactor = (double)width / IMAGE_WIDTH;
	m_dHightZoomFactor = (double)height / IMAGE_HEIGHT;
	m_zoomFactor = scale;
	ResizeImage();
	//opencv与MFC结合
	CWnd  *pWnd1 = GetDlgItem(IDC_PIC_STATIC);//CWnd是MFC窗口类的基类,提供了微软基础类库中所有窗口类的基本功能。
	pWnd1->SetWindowPos(NULL, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);
	CRect imgRect;
	pWnd1->GetClientRect(&imgRect);//GetClientRect为获得控件相自身的坐标大小
	namedWindow("src1", WINDOW_AUTOSIZE);//设置窗口名
	HWND hWndl = (HWND)cvGetWindowHandle("src1");//hWnd 表示窗口句柄,获取窗口句柄
	HWND hParent1 = ::GetParent(hWndl);//GetParent函数一个指定子窗口的父窗口句柄
	::SetParent(hWndl, GetDlgItem(IDC_PIC_STATIC)->m_hWnd);
	::ShowWindow(hParent1, SW_HIDE);//ShowWindow指定窗口中显示

	//开启标定线程
	g_vecTotalPoints.clear();
	g_vecPoints.clear();
	CWinThread *getROI_thread = AfxBeginThread(ThreadGetROI, this);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
