
#include "stdafx.h"
#include <dinput.h>
#include "resource.h"
#include "ConfigureInputDialog.h"
#include "core/Emulator.h"

//the total number of keys/buttons/axes the user can have in each binding
#define MAX_SUB_BINDINGS  5

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
  inputConfig(NULL)
{}

INT_PTR ConfigureInputDialog::AddBinding(InputConfiguration* inputConfig)
{
    if (inputConfig == NULL || inputConfig->bindingCount == MAX_BINDINGS)
        return IDCANCEL;

    this->inputConfig = inputConfig;

    return DoModal();
}

void ConfigureInputDialog::OnShowWindow(BOOL show, UINT i)
{
    CDialog::OnShowWindow(show, i);

    firstInputMark = 0;

    if (!show)
        return;

    GetInputLabel()->SetWindowText(inputConfig->inputObject->getName());
 
    //make space to hold our new binding
    inputConfig->producerIDs[inputConfig->bindingCount] = new GUID[MAX_SUB_BINDINGS];
    inputConfig->objectIDs[inputConfig->bindingCount] = new INT32[MAX_SUB_BINDINGS];
    inputConfig->subBindingCounts[inputConfig->bindingCount] = 0;

    //show the input we're trying to configure
    CStatic* list = GetInputLabel();
    list->SetWindowText(inputConfig->inputObject->getName());
    //take the focus off the button, so the user can configure a SPACE without triggering
    //the Cancel button
    list->SetFocus();

    //set up input manager
    manager = new InputProducerManager(m_hWnd);

    //acquire all the existing directinput devices
    IDirectInput8* directInput = manager->getDevice();
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
    INT32 bc = inputConfig->bindingCount;

    manager->pollInputProducers();
    int count = manager->getAcquiredInputProducerCount();
    for (int i = 0; i < count; i++) {
        InputProducer* nextProd = manager->getAcquiredInputProducer(i);
        INT32 inputCount = nextProd->getInputCount();
        for (INT32 j = 0; j < inputCount; j++) {
            float nextInput = nextProd->getValue(j);
            if (nextInput == 0.0f)
                continue;

            //check to be sure this input isn't a duplicate
            INT32 sbc = inputConfig->subBindingCounts[bc];
            BOOL foundDuplicate = FALSE;
            for (INT32 k = 0; k < sbc; k++) {
                if (inputConfig->producerIDs[bc][k] == nextProd->getGuid() &&
                        inputConfig->objectIDs[bc][k] == j)
                {
                    foundDuplicate = TRUE;
                    break;
                }
            }
            if (foundDuplicate)
                continue;

            inputConfig->producerIDs[bc][sbc] = nextProd->getGuid();
            inputConfig->objectIDs[bc][sbc] = j;
            inputConfig->subBindingCounts[bc]++;
            if (inputConfig->subBindingCounts[bc] == MAX_SUB_BINDINGS)
                break;

            if (firstInputMark == 0)
                firstInputMark = clock();
        }

        if (inputConfig->subBindingCounts[bc] == MAX_SUB_BINDINGS)
            break;
    }

    if (inputConfig->subBindingCounts[inputConfig->bindingCount] == MAX_SUB_BINDINGS ||
            (firstInputMark != 0 && (clock() - firstInputMark)/CLK_TCK > 0))
    {
        inputConfig->bindingCount++;
        EndDialog(IDOK);
    }
}

void ConfigureInputDialog::OnOK()
{
    //necessary to override this to prevent default behavior on using the ENTER key
}

void ConfigureInputDialog::OnCancel()
{
    delete[] inputConfig->producerIDs[inputConfig->bindingCount];
    delete[] inputConfig->objectIDs[inputConfig->bindingCount];
    EndDialog(IDCANCEL);
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

