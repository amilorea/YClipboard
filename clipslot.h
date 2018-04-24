#pragma once
#include "clippiece.h"
#include "control.h"

const int CLIPSLOT_MAXHOTKEYCOUNT		= 10;
const DWORD CLIPSLOT_DEFAULTSLOT		= -1;
const int CLIPSLOT_DISCARD				= 0x1;

const int CLIPSLOT_ERROR_EMPTYSLOT		= -0x1;
const int CLIPSLOT_ERROR_EMPTYCLIPPIECE	= -0x2;
const int CLIPSLOT_ERROR_NOCLIPPIECE	= -0x4;

class Clipslot {
private:
	std::vector<Clippiece*> clippieceVector;		// Chứa các clippiece
	std::vector<Clippiece*> clippieceSlotVector;	// Ánh xạ các clippiece đến vị trí hotkey
	int defaultSlot;								// Vị trí hotkey mặc định khi dán bằng cách thông thường
													// (Lệnh paste hoặc Ctrl + V)
	DWORD clippieceCount;
public:
	Clipslot();
	~Clipslot();
	int addSlot(Clippiece*);
	Clippiece* getClippiece(DWORD);
	Clippiece* getLastClippiece();
	void setLastClippiece(Clippiece*);
	void removeClippiece(DWORD);
	bool triggerClippieceSlot(DWORD);
	int createClippieceForCurrentClipboard();
	int size();
};

extern Clipslot _clipVector;
