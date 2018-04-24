#pragma once
#include "control.h"

const int CLIPPIECE_NOSLOT = -1;
const int CLIPPIECE_STATUS_READYTOUSE		= 0x1;
const int CLIPPIECE_STATUS_READYTODISCARD	= -0x1;

const int CLIPPIECE_ERROR_NOHANDLE	= -0x1;
const int CLIPPIECE_ERROR_NOFORMAT	= -0x2;
const int CLIPPIECE_ERROR_INITFAIL	= -0x4;

const int REMOVE_BUTTON_LOGICAL	= 0;
const int SLOT_BUTTON_LOGICAL	= 1;

class Clippiece {
private:
	std::map<UINT, HGLOBAL> dataMap;		// Map chứa cặp dữ liệu + format
	std::basic_string<WCHAR> des;			// Mô tả xem trước
	std::basic_string<WCHAR> desFull;		// Mô tả đầy đủ
	int status;								// Tình trạng hiện tại
	int slotOccupied;
	HWND removeButton;
	HWND slotButton;
public:
	Clippiece();
	~Clippiece();
	Clippiece(std::basic_string<WCHAR>);
	void setDes(std::basic_string<WCHAR>);
	std::basic_string<WCHAR> getDes();
	void setFullDes(std::basic_string<WCHAR>);
	std::basic_string<WCHAR> getFullDes();
	void setSlotInfo(int);
	int getSlotInfo();
	void setStatus(int);
	int getStatus();
	void setRemoveButton(HWND);
	HWND getRemoveButton();
	void setSlotButton(HWND);
	HWND getSlotButton();
	bool insertData(UINT, HGLOBAL);
	//void displayData();
	bool checkFormatAvailable(UINT);
	int getFormatCount();
	//HGLOBAL getData(UINT);
	bool injectAll();
	void destroyData();
	void removeAllButton();
};

bool setFileDescription(std::basic_string<WCHAR>&, std::basic_string<WCHAR>&);
bool setTextDescription(std::basic_string<WCHAR>&, std::basic_string<WCHAR>&);
bool setImageDescription(std::basic_string<WCHAR>&, std::basic_string<WCHAR>&);
bool setNoneDescription(std::basic_string<WCHAR>&, std::basic_string<WCHAR>&);