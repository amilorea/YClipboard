#pragma once
#include "stdafx.h"
#include "clipslot.h"
#include "WindowsProject1.h"
#include "network.h"

const int PAINTER_ERROR_FAIL_TO_VALIDATE			= 0x1;
const int PAINTER_ERROR_NONEXIST_SLOT				= 0x2;

const int PAINTER_WARNING_OUT_OF_X					= 0xF1;
const int PAINTER_WARNING_OUT_OF_Y					= 0xF2;

const int PAINTER_WARNING_BUTTON_EXISTED			= 0xFF3;

const int PAINTER_SCROLL_REMAIN_CURRENT_POSITION	= -1;	// Thanh cuộn giữ nguyên vị trí

const int PAINTER_MODE_NORMAL		= 0;	// Chế độ vẽ thông thường
const int PAINTER_MODE_CHANGESLOT	= 1;	// Chế độ vẽ khi đang trong quá trình đổi phím tắt

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

	static int downloadButtonWidth;
	static int downloadButtonHeight;
	static int downloadButtonMarginTop;
	static int downloadButtonMarginLeft;

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
	void addSlotButtonList();	// Thêm các nút bấm chuyển phím tắt
	void addSocketConnector();	// Thêm các nút bấm kết nối

	bool prepare();			// Chuẩn bị trước khi vẽ
	bool paint(HDC hdc);	// Vẽ
	bool redraw();			// Yêu cầu vẽ lại

	void setVerticalScroll(int newPosition = PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	// Đặt chiều cao mới cho thanh cuộn
	int updateVerticalScrollPosition(WORD position, int newPosition = PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	// Đặt vị trí mới cho thanh cuộn
	int getVerticalScrollPosition();	// Lấy vị trí thanh cuộn

	void setPaintMode(int);
	int getPaintMode();

	bool addSlotButton(int);
	bool addRemoveButton(int);
	bool addShareButton(int);
	void addDownloadButton();

	int clickEvent(WORD, WORD);
	int moveEvent(WORD, WORD);

	static void timedText(Painter *painter, Clippiece *clp);	// Hiển thị chữ trong một thời gian ngắn
};
extern Painter _painter;