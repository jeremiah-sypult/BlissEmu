#pragma once

#include <d3d9.h>
#include "ConfigureInputDialog.h"
#include "core/input/InputConsumer.h"
#include "core/input/InputConsumerObject.h"
#include "core/input/InputProducerManager.h"
#include "BlissConfiguration.h"
#include "DlgResizeHelper.h"

typedef struct _Binding Binding;

class BlissOptionsDialog : public CDialog
{
public:
    BlissOptionsDialog(CWnd* parent = NULL);

    BOOL OnInitDialog();
    afx_msg void OnShowWindow( BOOL, UINT );
    afx_msg void OnSelectionChanged(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnConfigureInput(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnOk();
    afx_msg void OnDestroy();
    afx_msg void OnConfigureAll();
    afx_msg void OnAddBinding();
    afx_msg void OnReset();
    afx_msg void OnResetAll();
    afx_msg void OnSize(UINT type, int cx, int cy);

private:
    void LoadConfiguredBindings(InputConsumer* inputConsumer);
    void SaveConfiguredBindings(InputConsumer* inputConsumer);
    void SetInputText(int itemIndex, InputConfiguration*);
    BOOL ConfigureInput();
    InputConsumer* GetSelectedInputConsumer();
    InputConsumerObject* GetSelectedInputObject();

    CTreeCtrl* GetTreeCtrl();
    CListCtrl* GetListCtrl();

    ConfigureInputDialog* configureInputDialog;
    InputProducerManager* manager;
    InputConfiguration** inputConfigs[5];
    INT32 inputObjectCounts[5];

    DlgResizeHelper resizeHelper;

	DECLARE_DYNAMIC(BlissOptionsDialog);
    DECLARE_MESSAGE_MAP();

};

