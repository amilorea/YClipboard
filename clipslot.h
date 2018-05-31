#pragma once
#include "clippiece.h"
#include "WindowsProject1.h"

const int CLIPSLOT_DISCARD					= 0x1;

const int CLIPSLOT_ERROR_EMPTYSLOT			= 0x1;	// Vị trí trống
const int CLIPSLOT_ERROR_EMPTY_CLIPPIECE	= 0x2;	// Clippiece trống
const int CLIPSLOT_ERROR_OUT_OF_RANGE		= 0x3;	// Vượt quá giới hạn cho phép
const int CLIPSLOT_ERROR_OCCUPIEDSLOT		= 0x4;	// Vị trí đã bị chiếm
const int CLIPSLOT_ERROR_CLIPPIECE			= 0x5;	// Lỗi không xác định ở phía clippiece

const int CLIPSLOT_LOGICAL_PADDING = 0xFF;

class Clipslot {
private:
	std::vector<Clippiece *> clippieceVector;				// Chứa các clippiece
	std::map<DWORD, std::pair<DWORD, char*> > sharingMap;	// Chứa mô tả của các clippiece đang chia sẻ

	static std::vector<HWND> slotButtonList;
	DWORD clippieceCount;
	DWORD hotkeySlotCount;

	DWORD requestClippiecePosition;	// Đây là số thứ tự logic của Clippiece đang thực hiện thao tác thay phím tắt

	int currentUniqueId;

	int addToClipslot(Clippiece*, DWORD position = CLIPPIECE_NOSLOT);	// Hàm can thiệp trực tiếp vào kho chứa
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

	bool requestChangeClippiecePosition(DWORD position);
	void cancelChangeClippiecePosition(DWORD position);
	DWORD getRequestClippiecePosition();

	DWORD changeClippiecePosition(DWORD slot);
	Clippiece* triggerClippieceSlot(DWORD slot);
	int createClippieceFromCurrentClipboard();
	int createClippieceFromSharedDescription(DWORD size, DWORD position, char* content);
	int addSharedDataToClippiece(DWORD size, char* data, DWORD position);

	void toggleShareSlot(DWORD position);	// Thay đổi các clippiece đang chia sẻ

	char* collectClippieceDescription(DWORD* size);				// Thu thập mô tả của các clippiece đang chia sẻ
	char* collectClippieceData(DWORD* size, DWORD position);	// Thu thập dữ liệu của clippiece đang chia sẻ
	void cleanSharedData();		// Dọn dẹp các clippiece được chia sẻ (khi ngắt kết nối)

	DWORD getLogicalPositionFromSharedClippiece(DWORD realPosition);

};
extern Clipslot _clipslot;
