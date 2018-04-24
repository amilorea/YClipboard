#pragma once
#include "stdafx.h"
#include "clipslot.h"

const int PAINTER_ERROR_FAILTOVALIDATE	= -0x1;
const int PAINTER_ERROR_NONEXISTSLOT	= -0x2;

const int PAINTER_WARNING_BUTTONEXISTED = 0x2;

class Painter {
private:
	static int clippieceHeight;
	static int clippieceWidth;
	static int removeButtonWidth;
	static int removeButtonHeight;
	static int slotButtonWidth;
	static int slotButtonHeight;
	static int previewMargin;
	static int clippieceMargin;
	static int removeButtonMarginTop;
	static int removeButtonMarginLeft;
	static int slotButtonMarginTop;
	static int slotButtonMarginLeft;
	DWORD lastStartSlot;
	DWORD lastEndSlot;
public:
	static int defaultWidth;
	static int defaultHeight;
	static int windowHeight;
	static int windowWidth;
	Painter();
	bool paintClippiece(HDC);
	void setVerticalScroll(int newPosition = 0);
	int updateVerticalScrollPosition(WORD, int newPosition = -1);
	int getVerticalScrollPosition();
	bool addSlotButton(int);
	bool addRemoveButton(int);
	bool redraw();
};

extern Painter _painter;