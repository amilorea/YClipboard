#pragma once
#include "stdafx.h"
enum PASTE_STATE { NOTHING, HOLD_CTRL, TYPE_V, HOLD_ALT, TYPE_NUMBER };

class Manager {
public:
	static const UINT CF_YCLIP;
	static PASTE_STATE pasteState;
	static HWND globalhWnd;
	static HFONT globalFontArial;
	static HFONT globalFontArialMedium;
	Manager();
};
