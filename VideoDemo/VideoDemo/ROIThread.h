#pragma once
#include <afxwin.h>
#include "PublicHeader.h"
#include "ROIDialog.h"

class ROIThread :
	public CWinThread
{
	DECLARE_DYNCREATE(ROIThread)
public:
	ROIThread();
	virtual ~ROIThread();
public:
	HWND m_hWnd;
	virtual int Run();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	ROIDialog m_dilogROI;
protected:
	DECLARE_MESSAGE_MAP()
};

