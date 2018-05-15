#pragma once
#include "clippiece.h"
#include "control.h"

const DWORD CLIPSLOT_DEFAULT_POSITION	= -1;
const int CLIPSLOT_DISCARD				= 0x1;

const int CLIPSLOT_ERROR_EMPTYSLOT			= 0x1;
const int CLIPSLOT_ERROR_EMPTY_CLIPPIECE	= 0x2;
const int CLIPSLOT_ERROR_OUT_OF_RANGE		= 0x4;
const int CLIPSLOT_ERROR_OCCUPIEDSLOT		= 0x8;

const int CLIPPIECE_POSITION_UNKNOWN = -1;

class Clipslot {
private:
	std::vector<Clippiece*> clippieceVector;		// Chứa các clippiece
	int defaultSlot;								// Vị trí hotkey mặc định khi dán bằng cách thông thường
													// (Lệnh paste hoặc Ctrl + V)
	static std::vector<HWND> slotButtonList;
	DWORD clippieceCount;
	DWORD hotkeySlotCount;
	DWORD requestClippiecePosition;

	int addToClipslot(Clippiece*, DWORD position = CLIPSLOT_DEFAULT_POSITION);	// Hàm can thiệp trực tiếp vào kho chứa

	// Hàm xử lý
	void copyBitmapInfo(PBITMAPINFO source, PBITMAPINFO destination);
	DWORD colorCalculate(WORD bitCount, DWORD colorUsed, DWORD compression);
public:
	Clipslot();
	~Clipslot();

	int size();
	int capacity();

	void setSlotButtonList(HWND);
	HWND getSlotButton(DWORD);

	// Dưới đây là các hàm can thiệp trực tiếp vào kho chứa
	// điều này có nghĩa là các hàm không cung cấp cơ chế soát lỗi logic
	Clippiece* getClippiece(DWORD);
	bool setClippiece(Clippiece*, DWORD);

	Clippiece* getLastClippiece();			
	void setLastClippiece(Clippiece*);

	bool swapClippiece(DWORD, DWORD);

	// Các hàm dưới đây can thiệp gián tiếp vào kho chứa, được bảo vệ bởi các cơ chế soát lỗi
	bool removeClippiece(DWORD);

	bool requestChangeClippiecePosition(DWORD);
	void cancelChangeClippiecePosition(DWORD);

	DWORD getRequestClippiecePosition();

	int changeClippiecePosition(DWORD);
	bool triggerClippieceSlot(DWORD);
	int createClippieceFromCurrentClipboard();
};
extern Clipslot _clipslot;
