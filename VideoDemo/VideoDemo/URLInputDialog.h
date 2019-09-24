#pragma once


// URLInputDialog 对话框

class URLInputDialog : public CDialogEx
{
	DECLARE_DYNAMIC(URLInputDialog)

public:
	URLInputDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~URLInputDialog();

	CString getInputUrl();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INPUT_URL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_url_input;
	CString m_input_url_text;
};
