#pragma once
#include "clippiece.h"
#include "WindowsProject1.h"

const int CLIPSLOT_DISCARD					= 0x1;

const int CLIPSLOT_ERROR_EMPTYSLOT			= 0x1;	// Vị trí trống
const int CLIPSLOT_ERROR_EMPTY_CLIPPIECE	= 0x2;	// Clippiece trống
const int CLIPSLOT_ERROR_OUT_OF_RANGE		= 0x3;	// Vượt quá giới hạn cho phép
const int CLIPSLOT_ERROR_OCCUPIEDSLOT		= 0x4;	// Vị trí đã bị chiếm
const int CLIPSLOT_ERROR_CLIPPIECE			= 0x5;	// Lỗi không xác định ở phía clippiece

const int CLIPSLOT_LOGICAL_PADDING			= 0xFF; // Hệ số bù cho vị trí logic, vì vị trí logic 0
													// có thể bị hiểu nhầm là NULL và bị bỏ qua

class Clipslot {
private:
	std::vector<Clippiece *> clippieceVector;				// Chứa các clippiece
	std::map<DWORD, std::pair<DWORD, char*> > sharingMap;	// Chứa mô tả của các clippiece đang chia sẻ

	static std::vector<HWND> slotButtonList;	// Các nút bấm chuyển hotkey
	DWORD clippieceCount;
	DWORD hotkeySlotCount;

	DWORD requestClippiecePosition;	// Đây là số thứ tự logic của Clippiece đang thực hiện thao tác thay phím tắt

	int currentUniqueId;			// Clippiece được tạo ra kế tiếp sẽ mang định danh này

	// Đẩy Clippiece vào kho chứa
	DWORD addToClipslot(Clippiece*, DWORD position = CLIPPIECE_NOSLOT);
public:
	Clipslot();
	~Clipslot();

	int size();			// Trả lại số Clippiece khả dụng
	int capacity();		// Trả lại số Clippiece trong kho

	void setSlotButtonList(HWND);
	HWND getSlotButton(DWORD);

	// Dưới đây là các hàm can thiệp trực tiếp vào kho chứa
	// điều này có nghĩa là các hàm không cung cấp cơ chế soát lỗi logic
	// Lấy / Thêm Clippiece
	Clippiece* getClippiece(DWORD);			
	bool setClippiece(Clippiece*, DWORD);

	// Lấy / Thêm Clippiece ở cuối dãy, phục vụ cho việc xóa
	Clippiece* getLastClippiece();			
	void setLastClippiece(Clippiece*);

	bool swapClippiece(DWORD, DWORD);	// Hoán đổi vị trí 2 Clippiece

	// Các hàm dưới đây can thiệp gián tiếp vào kho chứa, được bảo vệ bởi các cơ chế soát lỗi
	// Xóa Clippiece
	bool removeClippiece(DWORD);

	// Các hàm thực hiện việc đổi vị trí phím nóng
	bool requestChangeClippiecePosition(DWORD position);
	void cancelChangeClippiecePosition(DWORD position);
	DWORD getRequestClippiecePosition();
	DWORD changeClippiecePosition(DWORD slot);

	// Yêu cầu Clippiece đẩy dữ liệu vào clipboard
	Clippiece* triggerClippieceSlot(DWORD slot);

	// Các hàm tạo ra Clippiece mới từ dữ liệu cho trước
	int createClippieceFromCurrentClipboard();
	int createClippieceFromSharedDescription(DWORD size, DWORD position, char* content);
	int addSharedDataToClippiece(DWORD size, char* data, DWORD position);

	void toggleShareSlot(DWORD position);	// Thay đổi các clippiece đang chia sẻ

	char* collectClippieceDescription(DWORD* size);				// Thu thập mô tả của các clippiece đang chia sẻ
	char* collectClippieceData(DWORD* size, DWORD position);	// Thu thập dữ liệu của clippiece đang chia sẻ
	void cleanSharedData();		// Dọn dẹp các clippiece được chia sẻ (khi ngắt kết nối)

	// Lấy vị trí logic của Clippiece được chia sẻ trên máy chủ đang chia sẻ nó
	DWORD getLogicalPositionFromSharedClippiece(DWORD realPosition);
};
extern Clipslot _clipslot;
