#pragma once
#include "stdafx.h"
#include "clipslot.h"
#include "control.h"
#include "network.h"

const int PAINTER_ERROR_FAIL_TO_VALIDATE	= -0x1;
const int PAINTER_ERROR_NONEXIST_SLOT		= -0x2;

const int PAINTER_WARNING_OUT_OF_X			= -0x1;
const int PAINTER_WARNING_OUT_OF_Y			= -0x2;

const int PAINTER_WARNING_BUTTON_EXISTED	= 0x2;

const int PAINTER_SCROLL_REMAIN_CURRENT_POSITION = 0x01;

const int PAINTER_MODE_NORMAL = 0;
const int PAINTER_MODE_CHANGESLOT = 1;

class Painter {
private:
	static int clippieceHeight;
	static int clippieceWidth;

	static int removeButtonWidth;
	static int removeButtonHeight;
	static int removeButtonMarginTop;
	static int removeButtonMarginLeft;

	static int slotButtonWidth;
	static int slotButtonHeight;
	static int slotButtonMarginTop;
	static int slotButtonMarginLeft;

	static int shareButtonWidth;
	static int shareButtonHeight;
	static int copyButtonMarginTop;
	static int copyButtonMarginLeft;

	static int changeSlotButtonMarginLeft;
	static int changeSlotButtonMarginTop;

	static int previewMargin;
	static int clippieceMargin;

	static int ipBoxHeight;
	static int ipBoxWidth;
	static int connectButtonHeight;
	static int connectButtonWidth;
	static int connectorMarginTop;
	static int connectorMarginLeft;

	static HBRUSH unusedClippieceColor;
	DWORD startPosition;
	DWORD endPosition;
	int drawingPoint;
	int paintMode;
	int lastMousePosition;
public:
	static int minWindowWidth;
	static int minWindowHeight;
	static int maxWindowWidth;

	static int defaultWidth;
	static int defaultHeight;
	static int windowHeight;
	static int windowWidth;

	static HFONT globalFontArial;
	static HFONT globalFontArialMedium;
	static HCURSOR normalCursor;
	static HCURSOR pointerCursor;

	Painter();
	~Painter();
	void addSlotButtonList();
	void addSocketConnector();

	bool prepare();
	bool paint(HDC);
	bool redraw();

	void setVerticalScroll(int newPosition = PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	int updateVerticalScrollPosition(WORD, int newPosition = -1);
	int getVerticalScrollPosition();

	void setPaintMode(int);
	int getPaintMode();

	bool addSlotButton(int);
	bool addRemoveButton(int);
	bool addShareButton(int);

	int clickEvent(WORD, WORD);
	int moveEvent(WORD, WORD);
};

extern Painter _painter;