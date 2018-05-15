#pragma once
#include "stdafx.h"
enum PASTE_STATE { NOTHING, HOLD_CTRL, TYPE_V, HOLD_ALT, TYPE_NUMBER };

const int BUTTON_SKIP_RANGE			= 6;

const int SHARE_BUTTON				= 0;
const int UNSHARE_BUTTON			= 1;
const int REMOVE_BUTTON				= 2;
const int CHANGE_SLOT_BUTTON		= 3;
const int SELECT_SLOT_BUTTON		= 4;
const int CANCEL_CHANGE_SLOT_BUTTON = 5;
const int CONNECT_BUTTON			= 6;
const int DISCONNECT_BUTTON			 = 7;
const int IP_BOX = 8;

const int MAX_HOTKEY_COUNT = 10;

class Manager {
public:
	static const UINT CF_YCLIP;
	static PASTE_STATE pasteState;
	static HWND globalhWnd;
	Manager();
	static int resolveButtonType(WORD);
	static DWORD resolveButtonPosition(DWORD, int);
};