
#include "stdafx.h"
#include "resource.h"
#include "BlissOptionsDialog.h"
#include "BlissConfiguration.h"
#include "core/Emulator.h"
#include "BlissApp.h"

typedef struct _Binding
{
    InputProducer* inputProducer;
    INT32 inputEnum;
} Binding;

IMPLEMENT_DYNAMIC(BlissOptionsDialog, CDialog)

BEGIN_MESSAGE_MAP(BlissOptionsDialog, CDialog)
    ON_WM_SHOWWINDOW()
    ON_NOTIFY(TVN_SELCHANGED, IDC_OPTIONSTREE, BlissOptionsDialog::OnSelectionChanged)
    ON_NOTIFY(NM_DBLCLK, IDC_OPTIONSLIST, BlissOptionsDialog::OnConfigureInput)
    ON_NOTIFY(NM_RETURN, IDC_OPTIONSLIST, BlissOptionsDialog::OnConfigureInput)
    ON_COMMAND(IDOK, BlissOptionsDialog::OnOk)
    ON_COMMAND(IDCONFIGUREALL, BlissOptionsDialog::OnConfigureAll)
    ON_COMMAND(IDADDBINDING, BlissOptionsDialog::OnAddBinding)
    ON_COMMAND(IDRESET, BlissOptionsDialog::OnReset)
    ON_COMMAND(IDRESETALL, BlissOptionsDialog::OnResetAll)
    ON_WM_DESTROY()
    ON_WM_SIZE()
END_MESSAGE_MAP()

BlissOptionsDialog::BlissOptionsDialog(CWnd* parent)
: CDialog(IDD_OPTIONSDIALOG, parent),
  configureInputDialog(NULL),
  manager(NULL)
{
    memset(inputConfigs, 0, sizeof(inputConfigs));
    memset(inputObjectCounts, 0, sizeof(inputObjectCounts));
}

void BlissOptionsDialog::OnDestroy()
{
    for (INT32 i = 0; i < 3; i++) {
        if (inputConfigs[i] == NULL)
            continue;

        for (INT32 j = 0; j < inputObjectCounts[i]; j++) {
            if (inputConfigs[i][j] == NULL)
                continue;

            configuration.ReleaseInputConfiguration(inputConfigs[i][j]);
            inputConfigs[i][j] = NULL;
        }
        delete[] inputConfigs[i];
        inputConfigs[i] = NULL;
    }
    if (manager) {
        manager->releaseInputProducers();
        delete manager;
        manager = NULL;
    }
    if (configureInputDialog) {
        delete configureInputDialog;
        configureInputDialog = NULL;
    }
}

BOOL BlissOptionsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    HICON hIcon = theApp.LoadIcon(IDR_MAINFRAME);
    SetIcon(hIcon, TRUE);

    resizeHelper.Init(m_hWnd);
    resizeHelper.Fix(this->GetDlgItem(IDOK)->m_hWnd, DlgResizeHelper::kWidthRight, DlgResizeHelper::kHeightTop);
    resizeHelper.Fix(this->GetDlgItem(IDCANCEL)->m_hWnd, DlgResizeHelper::kWidthRight, DlgResizeHelper::kHeightTop);
    resizeHelper.Fix(this->GetDlgItem(IDADDBINDING)->m_hWnd, DlgResizeHelper::kWidthRight, DlgResizeHelper::kHeightTop);
    resizeHelper.Fix(this->GetDlgItem(IDRESET)->m_hWnd, DlgResizeHelper::kWidthRight, DlgResizeHelper::kHeightTop);
    resizeHelper.Fix(this->GetDlgItem(IDCONFIGUREALL)->m_hWnd, DlgResizeHelper::kWidthRight, DlgResizeHelper::kHeightTop);
    resizeHelper.Fix(this->GetDlgItem(IDRESETALL)->m_hWnd, DlgResizeHelper::kWidthRight, DlgResizeHelper::kHeightTop);
    resizeHelper.Fix(GetListCtrl()->m_hWnd, DlgResizeHelper::kRight, DlgResizeHelper::kTopBottom);
    resizeHelper.Fix(GetTreeCtrl()->m_hWnd, DlgResizeHelper::kNoHFix, DlgResizeHelper::kTopBottom);

    return TRUE;
}

void BlissOptionsDialog::OnShowWindow(BOOL b, UINT i)
{
    CDialog::OnShowWindow(b, i);

    CenterWindow();
    
    //create our input manager
    manager = new InputProducerManager(m_hWnd);

    //clear out the list control
    CListCtrl* list = GetListCtrl();
    list->DeleteAllItems();
    list->InsertColumn(0, "Input", LVCFMT_LEFT, 200);
    list->InsertColumn(1, "Binding", LVCFMT_LEFT, 200);

    //load the data into the tree and list views
    CTreeCtrl* tree = GetTreeCtrl();
    tree->DeleteAllItems();
    int emulatorCount = Emulator::GetEmulatorCount();
    TVINSERTSTRUCT nextItem;
    memset(&nextItem, 0, sizeof(nextItem));
    nextItem.hParent = TVI_ROOT;
    nextItem.hInsertAfter = NULL;
    nextItem.item.mask = TVIF_PARAM | TVIF_TEXT;
    int inputConsumerCounter = 0;
    for (int i = 0; i < emulatorCount; i++) {
        Emulator* nextEmu = Emulator::GetEmulator(i);
        UINT16 icCount = nextEmu->GetInputConsumerCount();
        for (UINT16 j = 0; j < icCount; j++) {
            InputConsumer* nextIc = nextEmu->GetInputConsumer(j);
            nextItem.item.pszText = (CHAR*)nextIc->getName();
            nextItem.item.lParam = (LPARAM)nextIc;
            nextItem.hInsertAfter = tree->InsertItem(&nextItem);
            if (j == 0)
                tree->SelectItem(nextItem.hInsertAfter);
            inputObjectCounts[inputConsumerCounter++] = nextIc->getInputConsumerObjectCount();
        }
        UINT32 pCount = nextEmu->GetPeripheralCount();
        for (UINT32 k = 0; k < pCount; k++) {
            Peripheral* nextPeriph = nextEmu->GetPeripheral(k);
            icCount = nextPeriph->GetInputConsumerCount();
            for (UINT16 j = 0; j < icCount; j++) {
                InputConsumer* nextIc = nextPeriph->GetInputConsumer(j);
                nextItem.item.pszText = (CHAR*)nextIc->getName();
                nextItem.item.lParam = (LPARAM)nextIc;
                nextItem.hInsertAfter = tree->InsertItem(&nextItem);
                inputObjectCounts[inputConsumerCounter++] = nextIc->getInputConsumerObjectCount();
            }
        }
    }

    tree->Expand(tree->GetRootItem(), TVE_EXPAND);
 }

void BlissOptionsDialog::OnConfigureInput(NMHDR*, LRESULT*)
{
    OnAddBinding();
}

void BlissOptionsDialog::OnSelectionChanged(NMHDR*, LRESULT*)
{
    CListCtrl* list = GetListCtrl();
    list->DeleteAllItems();
    CTreeCtrl* tree = GetTreeCtrl();
    TVITEM selectedItem;
    memset(&selectedItem, 0, sizeof(TVITEM));
    selectedItem.mask = TVIF_TEXT | TVIF_PARAM;
    selectedItem.hItem = tree->GetSelectedItem();
    if (!tree->GetItem(&selectedItem))
        return;

    InputConsumer* inputConsumer = (InputConsumer*)selectedItem.lParam;
    if (inputConsumer == NULL)
        return;

    INT32 id = inputConsumer->getId();
    if (inputConfigs[id] == NULL)
        LoadConfiguredBindings(inputConsumer);

    //display the configured bindings
    INT32 objectCount = inputConsumer->getInputConsumerObjectCount();
    for (int i = 0; i < objectCount; i++) {
        InputConsumerObject* nextObject = inputConsumer->getInputConsumerObject(i);
        LVITEM nextItem;
        memset(&nextItem, 0, sizeof(LVITEM));
        nextItem.mask = LVIF_PARAM | LVIF_TEXT;
        nextItem.iItem = i;
        nextItem.pszText = (CHAR*)nextObject->getName();
        nextItem.lParam = (LPARAM)nextObject;
        list->InsertItem(&nextItem);

        //display what the binding is
        SetInputText(i, inputConfigs[id][i]);
    }
}

void BlissOptionsDialog::LoadConfiguredBindings(InputConsumer* inputConsumer)
{
    //iterate through the input objects on this consumer
    INT32 id = inputConsumer->getId();
    if (inputConfigs[id] != NULL)
        return;

    INT32 objectCount = inputConsumer->getInputConsumerObjectCount();
    inputConfigs[id] = new InputConfiguration*[objectCount];
    memset(inputConfigs[id], 0, sizeof(InputConfiguration*)*objectCount);
    for (int i = 0; i < objectCount; i++) {
        InputConsumerObject* nextObject = inputConsumer->getInputConsumerObject(i);
        inputConfigs[id][i] = configuration.LoadInputConfiguration(inputConsumer, nextObject);
    }
}

void BlissOptionsDialog::OnConfigureAll()
{
    CListCtrl* list = GetListCtrl();
    int count = list->GetItemCount();
    for (int i = 0; i < count; i++) {
        list->SetItemState(i, LVIS_FOCUSED| LVIS_SELECTED, LVIS_FOCUSED| LVIS_SELECTED);
        if (!ConfigureInput())
            return;
    }
}

void BlissOptionsDialog::OnAddBinding()
{
    ConfigureInput();
}

InputConsumer* BlissOptionsDialog::GetSelectedInputConsumer()
{
    CTreeCtrl* tree = GetTreeCtrl();
    TVITEM selectedTreeItem;
    memset(&selectedTreeItem, 0, sizeof(TVITEM));
    selectedTreeItem.mask = TVIF_TEXT | TVIF_PARAM;
    selectedTreeItem.hItem = tree->GetSelectedItem();
    if (!tree->GetItem(&selectedTreeItem))
        return NULL;

    return (InputConsumer*)selectedTreeItem.lParam;
}

InputConsumerObject* BlissOptionsDialog::GetSelectedInputObject()
{
    CListCtrl* list = GetListCtrl();
    POSITION pos = list->GetFirstSelectedItemPosition();
    if (pos == NULL)
        return NULL;

    LVITEM selectedItem;
    memset(&selectedItem, 0, sizeof(TVITEM));
    selectedItem.mask = LVIF_PARAM | LVIF_TEXT;
    selectedItem.iItem = list->GetNextSelectedItem(pos);
    if (!list->GetItem(&selectedItem))
        return NULL;

    return (InputConsumerObject*)selectedItem.lParam;
}

BOOL BlissOptionsDialog::ConfigureInput()
{
    InputConsumer* inputConsumer = GetSelectedInputConsumer();
    if (inputConsumer == NULL)
        return FALSE;

    InputConsumerObject* inputObject = GetSelectedInputObject();
    if (inputObject == NULL)
        return FALSE;

    if (!configureInputDialog)
        configureInputDialog = new ConfigureInputDialog(this);

    INT32 cid = inputConsumer->getId();
    INT32 oid = inputObject->getId();
    InputConfiguration* inputConfig = inputConfigs[cid][oid];
    if (inputConfig == NULL) {
        inputConfig = new InputConfiguration();
        memset(inputConfig, 0, sizeof(InputConfiguration));
        inputConfig->inputObject = inputObject;
        inputConfigs[cid][oid] = inputConfig;
    }
    
    if (configureInputDialog->AddBinding(inputConfig) == IDCANCEL)
        return FALSE;

    CListCtrl* list = GetListCtrl();
    POSITION pos = list->GetFirstSelectedItemPosition();
    SetInputText(list->GetNextSelectedItem(pos), inputConfig);

    return TRUE;
}

void BlissOptionsDialog::OnResetAll()
{
    CListCtrl* list = GetListCtrl();
    int count = list->GetItemCount();
    for (int i = 0; i < count; i++) {
        list->SetItemState(i, LVIS_FOCUSED| LVIS_SELECTED, LVIS_FOCUSED| LVIS_SELECTED);
        OnReset();
    }
}

void BlissOptionsDialog::OnReset()
{
    InputConsumer* inputConsumer = GetSelectedInputConsumer();
    if (inputConsumer == NULL)
        return;

    InputConsumerObject* inputObject = GetSelectedInputObject();
    if (inputObject == NULL)
        return;

    INT32 cid = inputConsumer->getId();
    INT32 oid = inputObject->getId();
    if (inputConfigs[cid][oid] != NULL)
        configuration.ReleaseInputConfiguration(inputConfigs[cid][oid]);
    inputConfigs[cid][oid] = NULL;

    CListCtrl* list = GetListCtrl();
    POSITION pos = list->GetFirstSelectedItemPosition();
    SetInputText(list->GetNextSelectedItem(pos), NULL);
}

void BlissOptionsDialog::SetInputText(int itemIndex, InputConfiguration* inputConfig)
{
    if (inputConfig == NULL || inputConfig->bindingCount == 0) {
        GetListCtrl()->SetItemText(itemIndex, 1, "<not bound>");
        return;
    }

    //first need to iterate through and figure out how long our string will be
    size_t strLength = 1;  //room for the NULL at the end
    for (INT32 i = 0; i < inputConfig->bindingCount; i++) {
        if (inputConfig->bindingCount > 1)
            strLength += 2;  //parentheses around each binding if there is more than one
        for (INT32 j = 0; j < inputConfig->subBindingCounts[i]; j++) {
            InputProducer* producer = manager->acquireInputProducer(inputConfig->producerIDs[i][j]);
            if (producer == NULL)
                strLength += 15; //room for "<not connected>"
            else {
                if (!producer->isKeyboardDevice())
                    //room for the device name plus a "." (only if not a keyboard device)
                    strLength += strlen(producer->getName()) + strlen(producer->getInputName(inputConfig->objectIDs[i][j])) + 1;
                //room for the input name
                strLength += strlen(producer->getInputName(inputConfig->objectIDs[i][j]));
            }
            
            if (j+1 != inputConfig->subBindingCounts[i])
                strLength += 5; //room for " and "
        }

        if (i+1 != inputConfig->bindingCount)
            strLength += 4; //room for " or "
    }

    //form the textual representation of the binding
    CHAR* tmp = new CHAR[strLength];
    tmp[0] = NULL;
    for (INT32 i = 0; i < inputConfig->bindingCount; i++) {
        if (inputConfig->bindingCount > 1)
            strcat(tmp, "(");
        for (INT32 j = 0; j < inputConfig->subBindingCounts[i]; j++) {
            InputProducer* producer = manager->acquireInputProducer(inputConfig->producerIDs[i][j]);
            if (producer == NULL)
                strcat(tmp, "<not connected>");
            else {
                if (!producer->isKeyboardDevice()) {
                    strcat(tmp, producer->getName());
                    strcat(tmp, ".");
                }
                strcat(tmp, producer->getInputName(inputConfig->objectIDs[i][j]));
            }
            
            if (j+1 != inputConfig->subBindingCounts[i])
                strcat(tmp, " and ");
        }
        if (inputConfig->bindingCount > 1)
            strcat(tmp, ")");

        if (i+1 != inputConfig->bindingCount)
            strcat(tmp, " or ");
    }

    //display it
    GetListCtrl()->SetItemText(itemIndex, 1, tmp);
    delete[] tmp;
}

void BlissOptionsDialog::OnOk()
{
    //save all the input settings back to the registry
    int emulatorCount = Emulator::GetEmulatorCount();
    for (int i = 0; i < emulatorCount; i++) {
        Emulator* nextEmu = Emulator::GetEmulator(i);
        UINT16 icCount = nextEmu->GetInputConsumerCount();
        for (UINT16 j = 0; j < icCount; j++) {
            InputConsumer* nextIc = nextEmu->GetInputConsumer(j);
            SaveConfiguredBindings(nextIc);
        }

        UINT32 pCount = nextEmu->GetPeripheralCount();
        for (UINT32 k = 0; k < pCount; k++) {
            Peripheral* nextPeriph = nextEmu->GetPeripheral(k);
            icCount = nextPeriph->GetInputConsumerCount();
            for (UINT16 j = 0; j < icCount; j++) {
                InputConsumer* nextIc = nextPeriph->GetInputConsumer(j);
                SaveConfiguredBindings(nextIc);
            }
        }
    }

    EndDialog(IDOK);
}

void BlissOptionsDialog::SaveConfiguredBindings(InputConsumer* nextIc)
{
    //iterate through the list of input objects (emulated buttons, axes, etc.) on this consumer
    INT32 id = nextIc->getId();
    if (inputConfigs[id] == NULL)
        return;

    INT32 objectCount = nextIc->getInputConsumerObjectCount();
    for (INT32 i = 0; i < objectCount; i++) {
        configuration.SaveInputConfiguration(nextIc, nextIc->getInputConsumerObject(i), inputConfigs[id][i]);
        configuration.ReleaseInputConfiguration(inputConfigs[id][i]);
        inputConfigs[id][i] = NULL;
    }
}

void BlissOptionsDialog::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    resizeHelper.OnSize();
/*
    //adjust the components to fit. this is all hard-coded for now. we'll come up with something
    //more robust in the future
    CButton* nextButton = (CButton*)this->GetDlgItem(IDOK);
    if (nextButton != NULL)
        nextButton->MoveWindow(cx-119, 14, 10, 10);

    nextButton = (CButton*)this->GetDlgItem(IDCANCEL);
    if (nextButton != NULL)
        nextButton->MoveWindow(cx-119, 48, 10, 10);

    nextButton = (CButton*)this->GetDlgItem(IDADDBINDING);
    if (nextButton != NULL)
        nextButton->MoveWindow(cx-119, 82, 10, 10);

    nextButton = (CButton*)this->GetDlgItem(IDRESET);
    if (nextButton != NULL)
        nextButton->MoveWindow(cx-119, 116, 10, 10);

    nextButton = (CButton*)this->GetDlgItem(IDCONFIGUREALL);
    if (nextButton != NULL)
        nextButton->MoveWindow(cx-119, 150, 10, 10);

    nextButton = (CButton*)this->GetDlgItem(IDRESETALL);
    if (nextButton != NULL)
        nextButton->MoveWindow(cx-119, 184, 10, 10);

    CListCtrl* list = GetListCtrl();
    if (list != NULL)
        list->MoveWindow(10, 10, cx-324, cy-30);

    CTreeCtrl* tree = GetTreeCtrl();
    if (tree != NULL)
        tree->MoveWindow(10, 10, 179, cy-30);
*/
}

CTreeCtrl* BlissOptionsDialog::GetTreeCtrl()
{
    return (CTreeCtrl*)this->GetDlgItem(IDC_OPTIONSTREE);
}

CListCtrl* BlissOptionsDialog::GetListCtrl()
{
    return (CListCtrl*)this->GetDlgItem(IDC_OPTIONSLIST);
}

