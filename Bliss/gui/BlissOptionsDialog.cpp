
#include "stdafx.h"
#include "resource.h"
#include "BlissOptionsDialog.h"
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
    ON_WM_MBUTTONDBLCLK()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

BlissOptionsDialog::BlissOptionsDialog(CWnd* parent)
: CDialog(IDD_OPTIONSDIALOG, parent),
  configureInputDialog(NULL),
  manager(NULL)
{
  bindings[0] = NULL;
  bindings[1] = NULL;
  bindings[2] = NULL;
}

void BlissOptionsDialog::OnDestroy()
{
    if (bindings[0]) {
        delete[] bindings[0];
        bindings[0] = NULL;
    }
    if (bindings[1]) {
        delete[] bindings[1];
        bindings[1] = NULL;
    }
    if (bindings[2]) {
        delete[] bindings[2];
        bindings[2] = NULL;
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
                if (j == 0)
                    tree->SelectItem(nextItem.hInsertAfter);
            }
        }
    }

    tree->Expand(tree->GetRootItem(), TVE_EXPAND);
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
    if (!bindings[id])
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
        InputProducer* producer = bindings[id][i].inputProducer;
        if (producer == NULL) {
            list->SetItemText(i, 1, "<not bound>");
            continue;
        }
        INT32 e = bindings[id][i].inputEnum;
        if (e == -1) {
            list->SetItemText(i, 1, "<not bound>");
            continue;
        }

        CHAR* tmp = new CHAR[strlen(producer->getName()) + strlen(producer->getInputName(e)) + 2];
        strcpy(tmp, producer->getName());
        strcat(tmp, ".");
        strcat(tmp, producer->getInputName(e));
        list->SetItemText(i, 1, tmp);
        delete[] tmp;
    }
}

void BlissOptionsDialog::LoadConfiguredBindings(InputConsumer* inputConsumer)
{
    //iterator through the input objects on this consumer
    INT32 id = inputConsumer->getId();
    INT32 objectCount = inputConsumer->getInputConsumerObjectCount();
    if (!bindings[id]) {
        bindings[id] = new Binding[objectCount];
        memset(bindings[inputConsumer->getId()], 0, objectCount*sizeof(Binding));
    }

    for (int i = 0; i < objectCount; i++) {
        InputConsumerObject* nextObject = inputConsumer->getInputConsumerObject(i);

        //get the producer associated with this object
        InputProducer* producer = nextObject->getInputProducer();
        if (producer == NULL) {
            GUID g = nextObject->getDefaultDeviceGuid();
            CHAR* key = new CHAR[strlen(inputConsumer->getName()) + strlen(nextObject->getName()) + 13];
            strcpy(key, inputConsumer->getName());
            strcat(key, ".");
            strcat(key, nextObject->getName());
            strcat(key, ".DeviceGUID");
            UINT8* buffer = NULL;
            UINT32 size;
            theApp.GetProfileBinary("Input", key, &buffer, &size);
            delete[] key;
            if (buffer != NULL) {
                if (size == sizeof(GUID))
                    memcpy(&g, buffer, sizeof(GUID));
                delete[] buffer;
            }
            producer = manager->acquireInputProducer(g);
            if (producer == NULL)
                continue;
        }

        //now get the input enum associated with this object
        INT32 e = nextObject->getEnum();
        if (e < 0 || e >= producer->getInputCount()) { 
            e = nextObject->getDefaultEnum();
            CHAR* key = new CHAR[strlen(inputConsumer->getName()) + strlen(nextObject->getName()) + 12];
            strcpy(key, inputConsumer->getName());
            strcat(key, ".");
            strcat(key, nextObject->getName());
            strcat(key, ".InputEnum");
            e = theApp.GetProfileInt("Input", key, e);
            delete[] key;
            if (e < 0 || e >= producer->getInputCount())
                continue;
        }

        //bind the current settings to the input object
        bindings[inputConsumer->getId()][i].inputProducer = producer;
        bindings[inputConsumer->getId()][i].inputEnum = e;
    }
}

void BlissOptionsDialog::OnConfigureInput(NMHDR*, LRESULT*)
{
    CTreeCtrl* tree = GetTreeCtrl();
    TVITEM selectedTreeItem;
    memset(&selectedTreeItem, 0, sizeof(TVITEM));
    selectedTreeItem.mask = TVIF_TEXT | TVIF_PARAM;
    selectedTreeItem.hItem = tree->GetSelectedItem();
    if (!tree->GetItem(&selectedTreeItem))
        return;

    InputConsumer* inputConsumer = (InputConsumer*)selectedTreeItem.lParam;
    if (inputConsumer == NULL)
        return;

    CListCtrl* list = GetListCtrl();
    LVITEM selectedItem;
    memset(&selectedItem, 0, sizeof(TVITEM));
    selectedItem.mask = LVIF_PARAM | LVIF_TEXT;
    POSITION pos = list->GetFirstSelectedItemPosition();
    if (pos == NULL)
        return;
    selectedItem.iItem = list->GetNextSelectedItem(pos);
    if (!list->GetItem(&selectedItem))
        return;

    InputConsumerObject* inputObject = (InputConsumerObject*)selectedItem.lParam;
    if (inputObject == NULL)
        return;

    if (!configureInputDialog)
        configureInputDialog = new ConfigureInputDialog(this);

    configureInputDialog->SetInputLabel(inputObject->getName());
    if (configureInputDialog->DoModal() == IDCANCEL)
        return;

    GUID g = configureInputDialog->getConfiguredProducerGuid();
    InputProducer* producer = manager->acquireInputProducer(g);
    if (producer == NULL)
        return;

    INT32 e = configureInputDialog->getConfiguredEnum();
    if (e == -1)
        return;

    int i = inputObject->getId();
    bindings[inputConsumer->getId()][i].inputProducer = producer;
    bindings[inputConsumer->getId()][i].inputEnum = e;

    //display what the binding is
    CHAR* tmp = new CHAR[strlen(producer->getName()) + strlen(producer->getInputName(e)) + 2];
    strcpy(tmp, producer->getName());
    strcat(tmp, ".");
    strcat(tmp, producer->getInputName(e));
    list->SetItemText(selectedItem.iItem, 1, tmp);
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
    if (bindings[id] == NULL)
        return;

    int objCount = nextIc->getInputConsumerObjectCount();
    for (int k = 0; k < objCount; k++) {
        InputConsumerObject* nextObj = nextIc->getInputConsumerObject(k);

        //get the input producer (joystick, keyboard, etc.) bound to this object
        InputProducer* producer = bindings[id][k].inputProducer;
        if (producer != NULL) {
            //save the binding to the registry
            CHAR* key = new CHAR[strlen(nextIc->getName()) + strlen(nextObj->getName()) + 13];
            strcpy(key, nextIc->getName());
            strcat(key, ".");
            strcat(key, nextObj->getName());
            strcat(key, ".DeviceGUID");
            GUID g = producer->getGuid();
            theApp.WriteProfileBinary("Input", key, (LPBYTE)&g, sizeof(GUID));
            delete[] key;
        }

        //get the input enum (joystick axes, keyboard buttons, etc.) bound to this object
        INT32 e = bindings[id][k].inputEnum;
        if (e != -1) {
            //save the binding to the registry
            CHAR* key = new CHAR[strlen(nextIc->getName()) + strlen(nextObj->getName()) + 12];
            strcpy(key, nextIc->getName());
            strcat(key, ".");
            strcat(key, nextObj->getName());
            strcat(key, ".InputEnum");
            theApp.WriteProfileInt("Input", key, e);
            delete[] key;
        }
    }
}

CTreeCtrl* BlissOptionsDialog::GetTreeCtrl()
{
    return (CTreeCtrl*)this->GetDlgItem(IDC_OPTIONSTREE);
}

CListCtrl* BlissOptionsDialog::GetListCtrl()
{
    return (CListCtrl*)this->GetDlgItem(IDC_OPTIONSLIST);
}

