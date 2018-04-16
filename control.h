#pragma once
#include "stdafx.h"
enum PASTE_STATE { NOTHING, HOLD_CTRL, TYPE_V, HOLD_ALT, TYPE_NUMBER };

class Manager {
public:
	static UINT CF_YCLIP;
	static HWND globalhWnd;
	static PASTE_STATE pasteState;
	Manager();
};
