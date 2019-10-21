#pragma once
#include "PublicHeader.h"
#include "URLInputDialog.h"
#include "BlurThresDialog.h"

// MyDlg 对话框

class MyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MyDlg)

public:
	MyDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MyDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpenButton();

public:
	TCHAR* path;
	CEdit m_file;
	IplImage* TheImage;
	IplImage* StopImage;
	IplImage* RealImage;
	IplImage* CannyImage;
	CStatic m_video;
	CStatic m_image;
	CString strFilePath;
	CString strFileName;
	CvCapture* pCapture;//录制、拍摄
	cv::VideoCapture* vCap;
	int fps;
	int current_pos;
	int frames;
	bool scrolling = false;
	CString video_times;
	bool init = true;
	int init_c = 0;
	bool refresh = false;
	// CSliderCtrl m_video_slider;
	CSliderCtrl m_video_slider;

	//工作线程
	CWinThread* m_threadVideoCap;
	CWinThread* m_threadVideoDect;
	HANDLE hThreadSend;
	//其他窗体
	URLInputDialog urlin;
	BlurThresDialog thresdlg;
public:
	void InitialPic();
	void ShowImage(IplImage* image, UINT ID);
	void ResizeImage(IplImage* img, IplImage* to_img);
	void ResizeImage(IplImage* img);
	DWORD GetTimeFromServer(char* ip_addr);
	void ControlAllBtn(bool enable);
	void ProcessLoadVideo(CString path);
public:
	afx_msg void OnBnClickedBeginButton();
	afx_msg void OnBnClickedInsectButton();
	CString m_detect_type;
	void OnVideoChange(int n);
	void CloseVideo();
	void ButtomControl(bool open, bool start, bool stop, bool terminate);
	void DetectButtomControl(bool enable);
protected:
	afx_msg LRESULT ResultMessage(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedBulrButton();
	afx_msg void OnBnClickedDedustButton();
	CButton btn_open;
	CButton btn_begin;
	CButton btn_pause;
	CButton btn_terminate;
	CButton btn_insect_detect;
	CButton btn_blur_detect;
	CButton btn_dust_removal;
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedPauseButton();
	afx_msg void OnBnClickedStopButton();
	afx_msg void OnBnClickedHelmatDetect();
	afx_msg void OnBnClickedSmokeDetect();
	CStatic m_video_time_static;
	afx_msg void OnBnClickedOpenurlButton();
	CButton btn_url_open;
	CButton btn_hat_detect;
	CButton btn_smoke_detect;

	void OpenByJSWholeURL(CString url);
	void OpenByJSNecessaryText(int brand, int protocol, CString url_ip, int url_port, CString username, CString password, int channel, int subtype, int old_version, int codec);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	virtual BOOL OnInitDialog();
};
