#include "stdafx.h"
#include "ROIThread.h"

// ROIThread
IMPLEMENT_DYNCREATE(ROIThread, CWinThread)

ROIThread::ROIThread()
{
}

ROIThread::~ROIThread()
{
}

BOOL ROIThread::InitInstance()
{
	//�ڴ�ִ���������̳߳�ʼ��
	m_dilogROI.DoModal();
	return TRUE;
}

int ROIThread::ExitInstance()
{
	//�ڴ�ִ���������߳�����
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(ROIThread, CWinThread)
END_MESSAGE_MAP()

// ROIThread ��Ϣ�������
int ROIThread::Run() {
	::SendMessage(m_hWnd, WM_UPDARA_CONFIG, 0, 0);//�������淢����Ϣ�����������ļ�
	return CWinThread::Run();
}
