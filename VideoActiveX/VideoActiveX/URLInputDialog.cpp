// URLInputDialog.cpp: 实现文件
//

#include "pch.h"
#include "VideoActiveX.h"
#include "URLInputDialog.h"
#include "afxdialogex.h"


// URLInputDialog 对话框

IMPLEMENT_DYNAMIC(URLInputDialog, CDialogEx)

URLInputDialog::URLInputDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_URLINPUT_DIALOG, pParent)
	, m_input_url(_T(""))
	, m_c_url_v(FALSE)
	, m_text_ip(_T(""))
	, m_text_port(_T(""))
	, m_text_username(_T(""))
	, m_text_password(_T(""))
	, m_text_channel(_T(""))
{

}

URLInputDialog::~URLInputDialog()
{
}

void URLInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_URL_INPUT, m_input_url);
	DDX_Control(pDX, IDC_BRAND, m_cb_brand);
	DDX_Control(pDX, IDC_URL_TYPE, m_cb_urltype);
	DDX_Control(pDX, IDC_STREAM_TYPE, m_cb_streamtype);
	DDX_Control(pDX, IDC_CODEC, m_cb_codec);
	DDX_Control(pDX, IDC_RADIO1, m_c_url);
	DDX_Radio(pDX, IDC_RADIO1, m_c_url_v);
	DDX_Text(pDX, IDC_URL_IP, m_text_ip);
	DDX_Text(pDX, IDC_URL_PORT, m_text_port);
	DDX_Text(pDX, IDC_USERNAME, m_text_username);
	DDX_Text(pDX, IDC_PASSWORD, m_text_password);
	DDX_Text(pDX, IDC_CHANNALS, m_text_channel);
}


BEGIN_MESSAGE_MAP(URLInputDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &URLInputDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &URLInputDialog::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &URLInputDialog::OnBnClickedRadio2)
END_MESSAGE_MAP()


// URLInputDialog 消息处理程序
CString URLInputDialog::getInputUrl()
{
	return m_final_url;
}

void URLInputDialog::GenURL()
{
	UpdateData(true);
	if (m_c_url_v == 0)
	{
		m_final_url = m_input_url;
	}
	else
	{
		int index1 = m_cb_brand.GetCurSel();
		int index2 = m_cb_urltype.GetCurSel();
		//0360 1大华 2海康 3其他
		//大华格式：rtsp://username:password@ip:port/cam/realmonitor?channel=1&subtype=0
		switch (index1)
		{
		case 1:
			if (index2 == 1)
			{
				m_final_url = _T("rtmp://");
			}
			else if (index2 == 2)
			{
				m_final_url = _T("rtsp://");
			}
			m_final_url += m_text_username + ':' + m_text_password + '@' + m_text_ip + ':' + m_text_port + _T("/cam/realmonitor?channel=") + m_text_channel + _T("&subtype=") + (m_cb_streamtype.GetCurSel() == 1 ? '0' : '1');
			break;
		case 2:
			break;
		default:
			break;
		}
	}
}

void URLInputDialog::OnBnClickedOk()
{
	GenURL();
	CDialogEx::OnOK();
}


BOOL URLInputDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_cb_brand.AddString(_T("360"));   //添加字符串 
	m_cb_brand.AddString(_T("大华"));
	m_cb_brand.AddString(_T("海康"));
	m_cb_brand.AddString(_T("其他"));

	m_cb_urltype.AddString(_T("rtmp"));
	m_cb_urltype.AddString(_T("rtsp"));
	m_cb_urltype.AddString(_T("http"));

	m_cb_codec.AddString(_T("h264"));
	m_cb_codec.AddString(_T("MPEG-4"));
	m_cb_codec.AddString(_T("mpeg4"));

	m_cb_streamtype.AddString(_T("主码流"));
	m_cb_streamtype.AddString(_T("辅码流"));


	m_cb_brand.SetCurSel(1);   //默认选择第一个
	m_cb_urltype.SetCurSel(0);
	m_cb_codec.SetCurSel(0);
	m_cb_streamtype.SetCurSel(0);

	if (m_c_url_v == 0)
	{
		OnBnClickedRadio1();
	}
	else
	{
		OnBnClickedRadio2();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void URLInputDialog::OnBnClickedRadio1()
{
	UpdateData(true);
	GetDlgItem(IDC_URL_INPUT)->EnableWindow(true);
	Control_gen_url(false);
}


void URLInputDialog::OnBnClickedRadio2()
{
	UpdateData(true);
	GetDlgItem(IDC_URL_INPUT)->EnableWindow(false);
	Control_gen_url(true);
}

void URLInputDialog::Control_gen_url(bool enable)
{
	GetDlgItem(IDC_BRAND)->EnableWindow(enable);
	GetDlgItem(IDC_URL_TYPE)->EnableWindow(enable);
	GetDlgItem(IDC_STREAM_TYPE)->EnableWindow(enable);
	GetDlgItem(IDC_USERNAME)->EnableWindow(enable);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(enable);
	GetDlgItem(IDC_CODEC)->EnableWindow(enable);
	GetDlgItem(IDC_CHANNALS)->EnableWindow(enable);
	GetDlgItem(IDC_URL_PORT)->EnableWindow(enable);
	GetDlgItem(IDC_URL_IP)->EnableWindow(enable);
}
