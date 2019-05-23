#pragma once
#include "PublicHeader.h"

// ROIDialog 对话框

class ROIDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ROIDialog)

public:
	ROIDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ROIDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ROI_DIALOG };
#endif

private:
	double m_dWidthZoomFactor;
	double m_dHightZoomFactor;
	double m_zoomFactor;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSaveRoiButton();
	afx_msg void OnBnClickedClearButton();
	virtual BOOL OnInitDialog();
	void ResizeImage();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
