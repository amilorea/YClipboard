#include "stdafx.h"
#include "control.h"

const UINT Manager::CF_YCLIP = RegisterClipboardFormat(_T("YClipboard"));
PASTE_STATE Manager::pasteState = NOTHING;
HWND Manager::globalhWnd = 0;
HFONT Manager::globalFontArial = CreateFont(
	14,
	0,
	0,
	0,
	FW_DONTCARE,
	FALSE,
	FALSE,
	FALSE,
	DEFAULT_CHARSET,
	OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS,
	CLEARTYPE_QUALITY,
	VARIABLE_PITCH,
	TEXT("Arial"));
HFONT Manager::globalFontArialMedium = CreateFont(
	15,
	0,
	0,
	0,
	FW_DONTCARE,
	FALSE,
	FALSE,
	FALSE,
	DEFAULT_CHARSET,
	OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS,
	CLEARTYPE_QUALITY,
	VARIABLE_PITCH,
	TEXT("Arial"));

Manager::Manager()
{
}