// MainFrm.h : interface of the CBlissMainFrame class
//


#pragma once

#include <d3d9.h>
#include <mmreg.h>
#include <dsound.h>
#include "BlissAudioVideo.h"
#include "BlissOptionsDialog.h"
#include "BlissConfiguration.h"
#include "core/Emulator.h"
#include "core/input/InputProducerManager.h"

typedef enum _RunState {
    Stopped,
    Paused,
    Running
} RunState;

class BlissMainFrame : public CFrameWnd
{
	
public:
	BlissMainFrame();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    void SetFullScreen(BOOL fullScreen);
    void LoadAndRunRip(const CHAR* filename);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();

    //menu item handlers
	afx_msg void OnFileOpen();
	afx_msg void OnFileReset();
	afx_msg void OnFileClose();
	afx_msg void OnSettings();
	afx_msg void OnClose();
	afx_msg void OnPause();
	afx_msg void OnFullScreenMode();
    afx_msg void OnCheckMenuItems();
    afx_msg void OnCheckMenuItems(CCmdUI* pCmdUI);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnEnterMenuLoop(BOOL);
    afx_msg void OnExitMenuLoop(BOOL);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSetFocus(CWnd*);
    afx_msg void OnMouseMove(UINT, CPoint);
    afx_msg void OnTimer(UINT);

private:
    BOOL LoadRip(const CHAR*);
    BOOL LoadBinAsRip(const CHAR*);
    BOOL LoadRomAsRip(const CHAR*);
    BOOL SaveRip(const CHAR*);
    HRESULT InitializeDirect3D();
    HRESULT InitializeDirectSound(DWORD sampleRate);
    void InitializeDirectInput();
    BOOL InitializeEmulator();
    void InitializeEmulatorInputs();
    void InitializePeripheralInputs(Peripheral* periph);
	void Run();
    void ReleasePeripheralInputs(Peripheral* periph);
    void ReleaseEmulatorInputs();
    void ReleaseEmulator();
    void ReleaseDirect3D();
    void ReleaseDirectSound();
    void ReleaseDirectInput();
    BOOL ReacquireDevice();
    void SetPaused(BOOL paused);
    void BindInputConfiguration(InputConfiguration*, InputConsumerObject*);

    //configuration stuff
    LRESULT OnStartUp();
    void ShutDown();

	BOOL LoadPeripheralRoms(Peripheral* p);

    HACCEL accel;
	RunState runState;
	Emulator* currentEmu;
    Rip* currentRip;
    WINDOWPLACEMENT wp;
    BOOL menuInFullscreen;
    BOOL cursorShowing;

	AudioMixerWinDS* audioMixer;
	VideoBusWinD3D* videoBus;

	//DirectX stuff
	D3DPRESENT_PARAMETERS presentParams;
	IDirect3D9* direct3d;
	IDirect3DDevice9* direct3dDevice;
	IDirectSound8* directSound;
	IDirectSoundBuffer8* directSoundBuffer;
    InputProducerManager* manager;

    //subwindows
    CFileDialog* openDialog;
    BlissOptionsDialog* optionsDialog;

	DECLARE_DYNAMIC(BlissMainFrame)
	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};


