#pragma once

#include "core/input/InputProducerManager.h"

class ConfigureInputDialog : public CDialog
{
public:
    ConfigureInputDialog(CWnd* parent = NULL);

    virtual BOOL OnInitDialog();

    afx_msg void SetInputLabel(const CHAR* l);
    afx_msg void OnShowWindow(BOOL, UINT);
    afx_msg void OnTimer(UINT id);
    afx_msg void OnDestroy();
    afx_msg void OnKeyDown(UINT, UINT, UINT);
    afx_msg void OnKeyUp(UINT, UINT, UINT);

    GUID getConfiguredProducerGuid() { return configuredProducerGuid; }
    INT32 getConfiguredEnum() { return configuredEnum; }

private:
    CStatic* GetInputLabel();

    const CHAR* label;
    InputProducerManager* manager;
    GUID configuredProducerGuid;
    INT32 configuredEnum;

    DECLARE_DYNAMIC(ConfigureInputDialog);
    DECLARE_MESSAGE_MAP();

};