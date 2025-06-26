
// BFF.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CBFFApp:
// See BFF.cpp for the implementation of this class
//

class CBFFApp : public CWinApp
{
public:
	CBFFApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance(); // <--- Add this line

// Implementation
	ULONG_PTR m_gdiplusToken; // <--- Add this line

	DECLARE_MESSAGE_MAP()
};

extern CBFFApp theApp;
