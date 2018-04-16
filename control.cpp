#include "stdafx.h"
#include "control.h"

UINT Manager::CF_YCLIP = RegisterClipboardFormat(_T("YClipboard"));
PASTE_STATE Manager::pasteState = NOTHING;
HWND Manager::globalhWnd = 0;

Manager::Manager()
{
}
