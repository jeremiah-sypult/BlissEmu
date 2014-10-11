#pragma once

#include "BlissConfiguration.h"
#include "core/input/InputProducerManager.h"

class ConfigureInputDialog : public CDialog
{
public:
    ConfigureInputDialog(CWnd* parent = NULL);

    virtual BOOL OnInitDialog();

    INT_PTR AddBinding(InputConfiguration* inputConfig);
    void OnOK();
    void OnCancel();

    afx_msg void OnShowWindow(BOOL, UINT);
    afx_msg void OnTimer(UINT id);
    afx_msg void OnDestroy();
    afx_msg void OnKeyDown(UINT, UINT, UINT);
    afx_msg void OnKeyUp(UINT, UINT, UINT);

    InputConfiguration* GetInputConfiguration() { return inputConfig; }

private:
    CStatic* GetInputLabel();

    InputProducerManager* manager;
    InputConfiguration* inputConfig;
    clock_t firstInputMark;

    DECLARE_DYNAMIC(ConfigureInputDialog);
    DECLARE_MESSAGE_MAP();

};