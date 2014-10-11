// Bliss.Mfc.h : main header file for the Bliss.Mfc application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CBlissApp:
// See Bliss.Mfc.cpp for the implementation of this class
//

class BlissApp : public CWinApp
{
public:
	BlissApp();
	virtual BOOL InitInstance();
	virtual int ExitInstance();

    char StartUpPath[MAX_PATH];

	DECLARE_MESSAGE_MAP()

private:


};

extern BlissApp theApp;