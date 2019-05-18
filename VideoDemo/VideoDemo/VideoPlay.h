﻿#pragma once
#include "PublicHeader.h"

// VideoPlay 对话框

class VideoPlay : public CDialogEx
{
	DECLARE_DYNAMIC(VideoPlay)

public:
	VideoPlay(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~VideoPlay();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEO_PLAY_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCamButton();
	afx_msg void OnBnClickedPlayVideoButton();
	afx_msg void OnBnClickedCv340VideoButton();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedImshowButton();
	afx_msg void OnBnClickedResultButton();
	afx_msg void OnBnClickedHomoButton();
	afx_msg void OnBnClickedTrackButton();
	afx_msg void OnBnClickedMsrcrButton();

public:
	void showImage();

protected:
	cv::Mat frame;
public:
	afx_msg void OnBnClickedRoiButton();
};
