#pragma once

#include <d3d9.h>
#include "ConfigureInputDialog.h"
#include "core/input/InputConsumer.h"
#include "core/input/InputConsumerObject.h"
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
    afx_msg void OnConfigureAll();
    afx_msg void OnAddBinding();
    afx_msg void OnReset();

private:
    void LoadConfiguredBindings(InputConsumer* inputConsumer);
    void SaveConfiguredBindings(InputConsumer* inputConsumer);
    void SetInputText(int itemIndex, InputProducer* producer, INT32 e);
    BOOL ConfigureInput(InputConsumer* inputConsumer, InputConsumerObject* inputObject);
    InputConsumer* GetSelectedInputConsumer();
    InputConsumerObject* GetSelectedInputObject();

    CTreeCtrl* GetTreeCtrl();
    CListCtrl* GetListCtrl();

    ConfigureInputDialog* configureInputDialog;
    InputProducerManager* manager;
    Binding* bindings[3];

	DECLARE_DYNAMIC(BlissOptionsDialog);
    DECLARE_MESSAGE_MAP();

};

