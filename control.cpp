#include "stdafx.h"
#include "control.h"

const UINT Manager::CF_YCLIP = RegisterClipboardFormat(_T("YClipboard"));
PASTE_STATE Manager::pasteState = NOTHING;
HWND Manager::globalhWnd = 0;

Manager::Manager()
{
}

int Manager::resolveButtonType(WORD identifier)
{
	if (identifier < MAX_HOTKEY_COUNT) {
		return SELECT_SLOT_BUTTON;
	}
	else if (identifier == MAX_HOTKEY_COUNT) {
		return CANCEL_CHANGE_SLOT_BUTTON;
	}
	else if (identifier == (0xFF - 1)) {
		return CONNECT_BUTTON;
	}
	else if (identifier == (0xFF - 2)) {
		return DISCONNECT_BUTTON;
	}
	else if (identifier == (0xFF - 3)) {
		return IP_BOX;
	}
	else if(identifier >= 0xFF) {
		identifier = (identifier - 0xFF) % BUTTON_SKIP_RANGE;
		return identifier;
	}
	return 0;
}

DWORD Manager::resolveButtonPosition(DWORD identifier, int buttonType)
{
	switch (buttonType) {
	case SHARE_BUTTON:
	case REMOVE_BUTTON:
	case CHANGE_SLOT_BUTTON:
		return (identifier - 0xFF) / BUTTON_SKIP_RANGE;
	case SELECT_SLOT_BUTTON:
		return identifier;
	case CANCEL_CHANGE_SLOT_BUTTON:
		return MAX_HOTKEY_COUNT;
	}
	return -1;
}
