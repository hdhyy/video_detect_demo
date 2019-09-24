// VideoDemoDlg.h: 头文件
//
#pragma once
#include "ROIThread.h"
#include "PublicHeader.h"
#include "Detection.h"
#include "URLInputDialog.h"
#include "afxcmn.h"

// CVideoDemoDlg 对话框
class CVideoDemoDlg : public CDialogEx
{
// 构造
public:
	CVideoDemoDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEODEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
//线程
	//工作线程
	CWinThread* m_threadVideoCap;
	CWinThread* m_threadVideoDect;
	//界面线程
	ROIThread* m_threadGetROI;
	//ROI
	bool ReadROIFile(CString cstrFileName);
public:
	void InitialPic();
	void ShowImage(IplImage* image, UINT ID);
	void ResizeImage(IplImage* img, IplImage* to_img);
	void ResizeImage(IplImage* img);
	void OnVideoChange(int n);
	void ButtomControl(bool open, bool start, bool stop, bool terminate);
	void DisableAllDetectButton(bool b);
	void Canny();
	void CloseVideo();
	TCHAR *path;
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
	cv::VideoCapture *vCap;
	int fps;
	int current_pos;
	int frames;
	bool scrolling = false;
	CString video_times;
	URLInputDialog urlin;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedNightEnhanceButton();
	afx_msg void On32773();
	afx_msg void On32771();
	afx_msg void OnBnClickedVideoStartButton();
	afx_msg void OnBnClickedVideoStopButton();
	afx_msg void OnBnClickedVideoTerminateButton();
	CButton StopButton;
	CButton StartButton;
	afx_msg void On32774();
	afx_msg void OnBnClickedSafehatDetectButton();
	afx_msg void OnBnClickedSmokingDetectButton();
	CButton TerminateButton;
//	CSliderCtrl m_video_slider;
	CSliderCtrl m_video_slider;
	afx_msg void OnNMCustomdrawVideoSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnStnClickedVideoTimeStatic();
	CStatic m_video_time_static;
	afx_msg void OnBnClickedTestButton();
	afx_msg void OnBnClickedOpenButton();
	CButton OpenButton;
//	CEdit m_detect_type;
	CString m_detect_type;
	afx_msg void OnBnClickedSnatButton();
	afx_msg void OnBnClickedSetRoiButton();
	afx_msg void OnBnClickedShowRoiButton();
protected:
	afx_msg LRESULT ResultMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT UpdateROIData(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnHdhyy(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedTextureButton();
	afx_msg void OnBnClickedCornerButton();
	CButton SnatButton;
	CButton NightEnhanceButton;
	CButton TextureButton;
	CButton CornerButton;
	CButton HumanTraceButton;
	CButton HelmatButton;
	CButton SmokingButton;
	CButton ROISetButton;
	CButton ShowROIButton;
	afx_msg void OnBnClickedInsectButton();
	afx_msg void OnBnClickedBlurDetectButton();
	afx_msg void OnBnClickedFaceDetectButton();
	afx_msg void OnBnClickedHumanTrackButton();
	afx_msg void OnBnClickedDustButton();
	CButton InsectDetButton;
	CButton BlurDetButton;
	CButton FaceDetButton;
	CButton DeDustButton;
	afx_msg void OnBnClickedOpenurlButton();
};
