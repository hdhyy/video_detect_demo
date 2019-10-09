#pragma once


// URLInputDialog 对话框

class URLInputDialog : public CDialogEx
{
	DECLARE_DYNAMIC(URLInputDialog)

public:
	URLInputDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~URLInputDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_URLINPUT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_input_url;

	CString m_final_url;
	CString getInputUrl();

	afx_msg void OnBnClickedOk();
	CComboBox m_cb_brand;
	virtual BOOL OnInitDialog();
	CComboBox m_cb_urltype;
	CComboBox m_cb_streamtype;
	CComboBox m_cb_codec;
	CButton m_c_url;
	BOOL m_c_url_v;
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	void Control_gen_url(bool enable);
};
