
#include "stdafx.h"
#include "BlissApp.h"
#include "BlissMainFrame.h"
#include "BlissCommandLine.h"
#include "BlissCGC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(BlissApp, CWinApp)
END_MESSAGE_MAP()

BlissApp theApp;

BlissApp::BlissApp()
{}

BOOL BlissApp::InitInstance()
{
    // InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

    // save the application startup directory
    GetCurrentDirectory(MAX_PATH, StartUpPath);

	CWinApp::InitInstance();

    /*
    srand((unsigned int)time(NULL));
    CHAR test2[256];
    for (int i = 0; i < 100; i++) rand();
    sprintf(test2, "%08X", rand());
    MessageBox(NULL, test2, "ID", MB_OK);
    */

	// Standard initialization
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Bliss Intellivision Emulator"));

    // this code creates the frame window object and sets it as the
    // application's main window object
    BlissMainFrame* pFrame = new BlissMainFrame();
    m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
	HICON hIcon = LoadIcon(IDR_MAINFRAME);
	pFrame->SetIcon(hIcon, TRUE);

    // The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

    BlissCommandLine cmdInfo;
	ParseCommandLine(cmdInfo);
    pFrame->SetFullScreen(cmdInfo.m_bFullScreen);
    if (strlen(cmdInfo.m_strFileName))
        pFrame->LoadAndRunRip(cmdInfo.m_strFileName);
#if defined( CLASSIC_GAME_CONTROLLER )
    initCGC();
#endif
	return TRUE;
}

int BlissApp::ExitInstance()
{
#if defined( CLASSIC_GAME_CONTROLLER )
    releaseCGC();
#endif
    delete m_pMainWnd;
    m_pMainWnd = NULL;

    return CWinApp::ExitInstance();
}
