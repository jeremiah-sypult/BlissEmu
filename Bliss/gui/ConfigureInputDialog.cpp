
#include "stdafx.h"
#include <dinput.h>
#include "resource.h"
#include "ConfigureInputDialog.h"
#include "core/Emulator.h"

BOOL CALLBACK EnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

IMPLEMENT_DYNAMIC(ConfigureInputDialog, CDialog)

BEGIN_MESSAGE_MAP(ConfigureInputDialog, CDialog)
    ON_WM_SHOWWINDOW()
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
END_MESSAGE_MAP()

ConfigureInputDialog::ConfigureInputDialog(CWnd* parent)
: CDialog(IDD_CONFIGUREINPUTDIALOG, parent),
  manager(NULL),
  label(NULL)
{}

void ConfigureInputDialog::SetInputLabel(const CHAR* l)
{
    label = l;
}

void ConfigureInputDialog::OnShowWindow(BOOL show, UINT i)
{
    CDialog::OnShowWindow(show, i);

    firstInputMark = 0;

    if (!show)
        return;

    GetInputLabel()->SetWindowText(label);
 
    //reset the configured input
    memset(&configuredProducerGuid, 0, sizeof(GUID));
    configuredEnum = -1;

    //show the input we're trying to configure
    CStatic* list = GetInputLabel();
    list->SetWindowText(this->label);

    //set up input manager
    manager = new InputProducerManager(m_hWnd);

    //acquire all the existing directinput devices
    IDirectInput8* directInput = manager->getDevice();
    //directInput->EnumDevices(DI8DEVCLASS_GAMECTRL | DI8DEVCLASS_KEYBOARD, EnumDevices, this, DIEDFL_ATTACHEDONLY);
    directInput->EnumDevices(DI8DEVCLASS_ALL, EnumDevices, manager, DIEDFL_ATTACHEDONLY);
        
    //start polling timer
    this->SetTimer(1, 100, NULL);
}

void ConfigureInputDialog::OnDestroy()
{
    //stop the polling timer
    this->KillTimer(1);

    //release the input manager
    if (manager) {
        manager->releaseInputProducers();
        delete manager;
        manager = NULL;
    }
}

BOOL CALLBACK EnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    InputProducerManager* manager = (InputProducerManager*)pvRef;
    manager->acquireInputProducer(lpddi->guidInstance);

    return DIENUM_CONTINUE;
}

void ConfigureInputDialog::OnKeyDown(UINT nChar, UINT repCnt, UINT flags)
{
    if (nChar != VK_ESCAPE)
        return;

    CDialog::OnKeyDown(nChar, repCnt, flags);
}

void ConfigureInputDialog::OnKeyUp(UINT nChar, UINT repCnt, UINT flags)
{
    if (nChar != VK_ESCAPE)
        return;

    CDialog::OnKeyUp(nChar, repCnt, flags);
}

void ConfigureInputDialog::OnTimer(UINT)
{
    manager->pollInputProducers();
    int count = manager->getAcquiredInputProducerCount();
    for (int i = 0; i < count; i++) {
        InputProducer* nextProd = manager->getAcquiredInputProducer(i);
        INT32 input = nextProd->evaluateForAnyInput();
        if (input == -1)
            continue;

        this->configuredProducerGuid = nextProd->getGuid();
        this->configuredEnum = input;
        if (firstInputMark == 0)
            firstInputMark = clock();
        break;
    }

    if (firstInputMark != 0 && (clock() - firstInputMark)/CLK_TCK > 0)
        EndDialog(IDOK);
}

CStatic* ConfigureInputDialog::GetInputLabel()
{
    return (CStatic*)this->GetDlgItem(IDC_INPUTLABEL);
}

BOOL ConfigureInputDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    CenterWindow();

    return TRUE;
}

