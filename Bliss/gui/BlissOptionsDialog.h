#pragma once

#include <d3d9.h>
#include "ConfigureInputDialog.h"
#include "core/input/InputConsumer.h"
#include "core/input/InputProducerManager.h"

typedef struct _Binding Binding;

class BlissOptionsDialog : public CDialog
{
public:
    BlissOptionsDialog(CWnd* parent = NULL);

    afx_msg void OnShowWindow( BOOL, UINT );
    afx_msg void OnSelectionChanged(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnConfigureInput(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnOk();
    afx_msg void OnDestroy();

private:
    void LoadConfiguredBindings(InputConsumer* inputConsumer);
    void SaveConfiguredBindings(InputConsumer* inputConsumer);

    CTreeCtrl* GetTreeCtrl();
    CListCtrl* GetListCtrl();

    ConfigureInputDialog* configureInputDialog;
    InputProducerManager* manager;
    Binding* bindings[3];

	DECLARE_DYNAMIC(BlissOptionsDialog);
    DECLARE_MESSAGE_MAP();

};

