#pragma once


// BlurThresDialog 对话框

class BlurThresDialog : public CDialogEx
{
	DECLARE_DYNAMIC(BlurThresDialog)

public:
	BlurThresDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~BlurThresDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BLUR_THRESHOLD_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cb_blur_thres;
	int thres_index = 6;
	float threshold;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
