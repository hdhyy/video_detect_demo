
// MFCObjectTracking190513.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMFCObjectTracking190513App:
// See MFCObjectTracking190513.cpp for the implementation of this class
//

class CMFCObjectTracking190513App : public CWinApp
{
public:
	CMFCObjectTracking190513App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMFCObjectTracking190513App theApp;
