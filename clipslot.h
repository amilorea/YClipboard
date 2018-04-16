#pragma once
#include "clippiece.h"
#include "control.h"

const int CLIPSLOT_MAXSIZE				= 10;
const DWORD CLIPSLOT_DEFAULTSLOT		= -1;

const int CLIPSLOT_ERROR_EMPTYSLOT		= -1;
const int CLIPSLOT_ERROR_EMPTYCLIPPIECE	= -2;

class Clipslot {
private:
	std::vector<Clippiece*> clippieceVector;		// Chứa các clippiece
	std::vector<Clippiece*> clippieceSlotVector;	// Ánh xạ các clippiece đến vị trí hotkey
	int defaultSlot;								// Vị trí hotkey mặc định khi dán bằng cách thông thường
													// (Lệnh paste hoặc Ctrl + V)
public:
	Clipslot();
	~Clipslot();
	int addSlot(Clippiece*);
	bool triggerClippieceSlot(DWORD);
	bool createClippieceForCurrentClipboard();
};

extern Clipslot _clipVector;
