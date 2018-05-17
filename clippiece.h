#pragma once
#include "control.h"

const int CLIPPIECE_DEFAULT_DESCRIPTION_SIZE = 100;

const int CLIPPIECE_NOSLOT					= 0x7FFF - 1;
const int CLIPPIECE_STATUS_READY_TO_USE		= 0x0;
const int CLIPPIECE_STATUS_READY_TO_DISCARD	= 0x1;

const int CLIPPIECE_ERROR_NO_HANDLE		= -0x1;
const int CLIPPIECE_ERROR_NO_FORMAT		= -0x2;
const int CLIPPIECE_ERROR_INIT_FAILED	= -0x4;

class Clippiece {
private:
	std::map<UINT, HGLOBAL> dataMap;		// Map chứa cặp dữ liệu + format
	std::basic_string<wchar_t> des;			// Mô tả xem trước
	std::basic_string<wchar_t> desFull;		// Mô tả đầy đủ
	int status;								// Tình trạng hiện tại
	int slotOccupied;

	HWND removeButton;
	HWND slotButton;
	HWND shareButton;

	bool setTimeDescription();
	bool setFileDescription(bool flag);
	bool setTextDescription(bool flag);
	bool setImageDescription(bool flag);
	bool setNoneDescription();
public:
	Clippiece();
	Clippiece(std::basic_string<wchar_t>);
	~Clippiece();

	void destroyData();
	void removeAllButton();

	void setDes(std::basic_string<wchar_t>);
	std::basic_string<wchar_t> getDes();
	void setFullDes(std::basic_string<wchar_t>);
	std::basic_string<wchar_t> getFullDes();

	void setPosition(int, bool isAffectHotkey = false);
	int getPosition();

	void setStatus(int);
	int getStatus();

	void setRemoveButton(HWND);
	HWND getRemoveButton();

	void setSlotButton(HWND);
	HWND getSlotButton();

	void setShareButton(HWND);
	HWND getShareButton();

	bool insertData(UINT, HGLOBAL);
	//void displayData();
	bool isFormatAvailable(UINT);
	int countFormat();
	//HGLOBAL getData(UINT);
	bool injectAll();

	void setDesscription();
};