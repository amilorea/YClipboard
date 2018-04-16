#pragma once
#include "stdafx.h"

const int CLIPPIECE_NOSLOT = -1;

const int CLIPPIECE_ERROR_NOHANDLE	= -1;
const int CLIPPIECE_ERROR_NOFORMAT	= -2;
const int CLIPPIECE_ERROR_INITFAIL	= -3;

class Clippiece {
private:
	std::map<UINT, HGLOBAL> dataMap;		// Map chứa cặp dữ liệu + format
	std::basic_string<WCHAR> des;			// Mô tả xem trước
	std::basic_string<WCHAR> desFull;		// Mô tả đầy đủ
	int status;								// Tình trạng hiện tại
	int slotOccupied;
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
	bool insertData(UINT, HGLOBAL);
	//void displayData();
	bool checkFormatAvailable(UINT);
	int getFormatCount();
	//HGLOBAL getData(UINT);
	bool injectAll();
};

bool setFileDescription(std::basic_string<WCHAR>, std::basic_string<WCHAR>);
bool setTextDescription(std::basic_string<WCHAR>, std::basic_string<WCHAR>);
bool setImageDescription(std::basic_string<WCHAR>, std::basic_string<WCHAR>);
bool setNoneDescription(std::basic_string<WCHAR>, std::basic_string<WCHAR>);