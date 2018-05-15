#include "stdafx.h"
#include "clippiece.h"
#include "control.h"

Clippiece::Clippiece()
{
	slotOccupied = CLIPPIECE_NOSLOT;
	status = CLIPPIECE_STATUS_READY_TO_USE;
	removeButton = NULL;
	slotButton = NULL;
	copyButton = NULL;
}

Clippiece::Clippiece(std::basic_string<wchar_t> des)
{
	slotOccupied = CLIPPIECE_NOSLOT;
	status = CLIPPIECE_STATUS_READY_TO_USE;
	setFullDes(des);
	des.resize(min(CLIPPIECE_DEFAULT_DESCRIPTION_SIZE, des.size()));
	setDes(des);
	removeButton = NULL;
	slotButton = NULL;
}

Clippiece::~Clippiece()
{
	consoleOutput(_T("Destroying clippiece at position"), CONSOLE_SPACE);
	consoleOutput(getPosition(), CONSOLE_NEWLINE);

	destroyData();
	removeAllButton();
	std::map<UINT, HGLOBAL>::iterator it = dataMap.find(Manager::CF_YCLIP);
	if (it != dataMap.end()) {
		GlobalFree(it->second);
	}
	dataMap.clear();
}

void Clippiece::destroyData() {
	des.clear();
	desFull.clear();
	for (std::map<UINT, HGLOBAL>::const_iterator it = dataMap.cbegin(); it != dataMap.cend();) {
		// Phá hủy tất cả dữ liệu trừ CF_YCLIP để chặn việc thêm Clippiece đệ quy
		if (it->second != NULL && it->second != NULL && it->first != Manager::CF_YCLIP) {
			GlobalFree(it->second);
			it = dataMap.erase(it);
		}
		else it++;
	}
	
	consoleOutput(_T("Successfully destroyed data"), CONSOLE_NEWLINE);
}

void Clippiece::removeAllButton()
{
	if (removeButton != NULL) {
		DestroyWindow(removeButton);
		removeButton = NULL;
	}
	if (slotButton != NULL) {
		DestroyWindow(slotButton);
		slotButton = NULL;
	}
	if (copyButton != NULL) {
		DestroyWindow(copyButton);
		copyButton = NULL;
	}
	
	consoleOutput(_T("Successfully removed buttons"), CONSOLE_NEWLINE);
}

void Clippiece::setDes(std::basic_string<wchar_t> des)
{
	des.resize(min(CLIPPIECE_DEFAULT_DESCRIPTION_SIZE, des.size()));
	this->des = des;

	consoleOutput(_T("Description set"), CONSOLE_NEWLINE);
}

std::basic_string<wchar_t> Clippiece::getDes()
{
	return des;
}

void Clippiece::setFullDes(std::basic_string<wchar_t> des)
{
	desFull = des;
	
	consoleOutput(_T("Full Description set"), CONSOLE_NEWLINE);
}

std::basic_string<wchar_t> Clippiece::getFullDes()
{
	return desFull;
}

void Clippiece::setPosition(int slot, bool isAffectHotkey)
{
	slotOccupied = slot;
	// Thay đổi nút xóa
	HWND tempRemoveButton = getRemoveButton();
	if (tempRemoveButton != NULL) {
		// Hàm SetWindowLongPtr dùng thay đổi định danh của nút bấm, dùng cho cửa sổ con
		SetWindowLongPtr(tempRemoveButton, GWLP_ID, (0xFF + slot * BUTTON_SKIP_RANGE + REMOVE_BUTTON));
	}

	// Thay đổi nút chuyển hotkey
	HWND tempSlotButton = getSlotButton();
	if (tempSlotButton != NULL) {
		SetWindowLongPtr(tempSlotButton, GWLP_ID, (0xFF + slot * BUTTON_SKIP_RANGE + CHANGE_SLOT_BUTTON));
		std::basic_string<wchar_t> buttonText;
		if (isAffectHotkey) {
			buttonText.append(_T("Ctrl "));
			buttonText.push_back(slot + '0');
		}
		else {
			buttonText.append(_T("Hotkey"));
		}
		SendMessage(tempSlotButton, WM_SETTEXT, 0, (LPARAM)buttonText.c_str());
	}

	// Thay đổi nút xóa
	HWND tempCopyButton = getShareButton();
	if (tempCopyButton != NULL) {
		// Hàm SetWindowLongPtr dùng thay đổi định danh của nút bấm, dùng cho cửa sổ con
		SetWindowLongPtr(tempCopyButton, GWLP_ID, (0xFF + slot * BUTTON_SKIP_RANGE + SHARE_BUTTON));
	}
	
	consoleOutput(_T("Succesfully set slot to"), CONSOLE_SPACE);
	consoleOutput(getPosition(), CONSOLE_NEWLINE);
}

int Clippiece::getPosition()
{
	return slotOccupied;
}

void Clippiece::setStatus(int status)
{
	this->status = status;
}

int Clippiece::getStatus()
{
	return status;
}

void Clippiece::setRemoveButton(HWND button)
{
	removeButton = button;
}

HWND Clippiece::getRemoveButton()
{
	return removeButton;
}

void Clippiece::setSlotButton(HWND button)
{
	slotButton = button;
}

HWND Clippiece::getSlotButton()
{
	return slotButton;
}

void Clippiece::setShareButton(HWND button)
{
	copyButton = button;
}

HWND Clippiece::getShareButton()
{
	return copyButton;
}

bool Clippiece::insertData(UINT format, HGLOBAL handle)
{
	try {
		if (handle == NULL)
			throw CLIPPIECE_ERROR_NO_HANDLE;

		if (format < 0)
			throw CLIPPIECE_ERROR_NO_FORMAT;

		dataMap.insert(std::pair<UINT, HGLOBAL>(format, handle));

		consoleOutput(_T("Clippiece insert succeed with format "), CONSOLE_NORMAL);
		consoleOutput(format, CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPPIECE_ERROR_NO_HANDLE:
			consoleOutput(_T("Clippiece insert failed - Error: Data handle is NULL "), CONSOLE_NEWLINE);
			break;
		case CLIPPIECE_ERROR_NO_FORMAT:
			consoleOutput(_T("Clippiece insert failed - Error: No such format"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

//void Clippiece::display()
//{
//	if (_debug) {
//		consoleOutput(_T("Clippice slot "), 0);
//		consoleOutput(this->slot, 1);
//		consoleOutput(des.c_str(), 1);
//		consoleOutput(desFull.c_str(), 1);
//		wchar_t szFormatName[100];						// Tên của format
//		HGLOBAL hContentLock = NULL;
//		for (std::map<UINT, HGLOBAL>::iterator it = m.begin(); it != m.end(); it++) {
//			consoleOutput(_T("["), 0);
//			consoleOutput(it->first, 0);
//			consoleOutput(_T("]"), 0);
//			consoleOutput(_T("["), 0);
//			if (GetClipboardFormatName(it->first, szFormatName, sizeof(szFormatName)))
//				consoleOutput(szFormatName, 0);
//			else
//				consoleOutput(_T("unknown"), 0);
//			consoleOutput(_T("]: "), 0);
//
//			try {
//				if (it->second) {
//					hContentLock = (HGLOBAL)GlobalLock(it->second);
//					if (hContentLock != NULL) {
//						SIZE_T size = GlobalSize(it->second);	// Kích thước của nội dung clipboard
//						if (_debug) {
//							consoleOutput(_T("Size: "), 0);
//							consoleOutput(size, 1);
//						}
//						GlobalUnlock(it->second);
//					}
//					else {
//						consoleOutput(_T("Clippiece display: Error trying to lock, Error Code "), 1);
//						consoleOutput(GetLastError(), 1);
//					}
//				}
//			}
//			catch (int error) {
//				if (hContentLock != NULL) {
//					GlobalUnlock(it->second);
//				}
//				consoleOutput(GetLastError(), 1);
//				throw;
//			}
//		}
//	}
//}

bool Clippiece::isFormatAvailable(UINT format)
{
	return dataMap.find(format) != dataMap.end();
}

int Clippiece::countFormat()
{
	return dataMap.size();
}

//HGLOBAL Clippiece::getData(UINT f)
//{
//	if (dataMap.find(f) != m.end()) {
//		HGLOBAL hTemp = m.find(f)->second;
//		SIZE_T size = GlobalSize(hTemp);
//		HGLOBAL hCopy = GlobalAlloc(GPTR, size);
//		HGLOBAL hData;
//		try {
//			hData = GlobalLock(hTemp);
//			if (hCopy != NULL) {
//				memcpy(hCopy, hData, size);
//			}
//			GlobalUnlock(hTemp);
//		}
//		catch (int error) {
//			if (hData) {
//				GlobalUnlock(hTemp);
//			}
//		}
//		return hCopy;
//	}
//	return nullptr;
//}

bool Clippiece::injectAll()
{
	try {
		consoleOutput(_T("Attempting to set clipboard data from slot "), CONSOLE_NORMAL);
		consoleOutput(slotOccupied, 1);

		if (!OpenClipboard(Manager::globalhWnd))
			throw CLIPBOARD_ERROR_NOT_OPEN;
		
		EmptyClipboard();		// Gọi hàm Emptyclipboard sẽ lấy quyền sở hữu clipboard về cho chương trình
		for (std::map<UINT, HGLOBAL>::iterator it = dataMap.begin(); it != dataMap.end(); it++) {
			UINT format = it->first;
			HGLOBAL hTemp = it->second;
			if (hTemp == NULL) {
				consoleOutput(_T("Clippiece inject - Warning: No handle available at format "), CONSOLE_NORMAL);
				consoleOutput(format, CONSOLE_NEWLINE);

				continue;
			}

			// Clipboard lấy quyền sở hữu của dữ liệu sau khi nhận và có thể sẽ xóa các dữ liệu này nếu cần
			// Do đó ta không gửi trực tiếp dữ liệu từ clippiece, mà chỉ gửi một bản sao của nó
			SIZE_T dataSize = GlobalSize(hTemp);

			consoleOutput(_T("Clippiece inject - Data size:"), CONSOLE_SPACE);
			consoleOutput(dataSize, CONSOLE_NEWLINE);

			HGLOBAL hDataCopy = GlobalAlloc(GPTR, dataSize);
			HGLOBAL hDataLock;
			try {
				hDataLock = GlobalLock(hTemp);
				if (hDataCopy == NULL)
					throw ALLOCATION_FAILED;

				if (hDataLock == NULL)
					throw GLOBALLOCK_FAILED;

				memcpy(hDataCopy, hDataLock, dataSize);

				GlobalUnlock(hTemp);
				SetClipboardData(it->first, hDataCopy);
			}
			catch (int error) {
				switch (error) {
				case ALLOCATION_FAILED:
					consoleOutput(_T("Clippiece inject - Error: Unable to copy data at format "), CONSOLE_NORMAL);
					consoleOutput(error, it->first);
					break;
				case GLOBALLOCK_FAILED:
					consoleOutput(_T("Clippiece inject - Error: Unable to lock data at format "), CONSOLE_NORMAL);
					consoleOutput(error, it->first);
					break;
				}

				if (hDataLock)
					GlobalUnlock(hTemp);
			}
		}
		CloseClipboard();

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_NOT_OPEN:
			consoleOutput(_T("Clippiece inject failed - Error: Unable to open clipboard"), CONSOLE_NEWLINE);
			return false;
		}
	}
	return false;
}

bool setFileDescription(std::basic_string<wchar_t> &des, std::basic_string<wchar_t> &desFull)
{
	HANDLE hFileContent = (HANDLE)GetClipboardData(CF_HDROP);
	HDROP hFileLock = NULL;
	UINT totalFile = 0;
	bool firstOccur = true;
	try {
		if (hFileContent == NULL)
			throw CLIPBOARD_ERROR_DATA_NOT_FOUND;

		hFileLock = (HDROP)GlobalLock(hFileContent);
		if (hFileLock == NULL)
			throw GLOBALLOCK_FAILED;

		totalFile = DragQueryFile(hFileLock, 0xFFFFFFFF, nullptr, 0);	// Cú pháp đặc trưng của DragQueryFile dùng lấy số tệp

		if (totalFile == 0)
			throw FORMAT_ERROR_NO_FILE;

		desFull.append(std::to_wstring(totalFile));
		desFull.append(_T(" tệp / thư mục: "));

		des.append(std::to_wstring(totalFile));
		des.append(_T(" tệp / thư mục:"));

		for (DWORD fileCounter = 0; fileCounter < totalFile; fileCounter++) {
			std::basic_string<wchar_t> fileName;
			UINT fileNameLength = DragQueryFile(hFileLock, fileCounter, nullptr, 0);	// Cú pháp lấy độ dài tên tệp
			fileName.resize(fileNameLength + 1);										// Tính thêm ký tự \0 kết thúc xâu
			DragQueryFile(hFileLock, fileCounter, &(fileName[0]), fileNameLength + 1);	// Cú pháp ghi lại tên tệp

			desFull.append(_T("\n\t"));
			desFull.append(fileName.c_str());

			if (firstOccur) {
				des.append(_T(" "));
				firstOccur = false;
			} else des.append(_T(", "));
			des.append(fileName.substr(fileName.find_last_of('\\') + 1).c_str());
		}

		GlobalUnlock(hFileLock);

		consoleOutput(_T("Description by CF_HDROP sucessfully created"), CONSOLE_NEWLINE);
		consoleOutput(des.c_str(), CONSOLE_NEWLINE);
		consoleOutput(desFull.c_str(), CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_DATA_NOT_FOUND:
			consoleOutput(_T("Clippiece create description by CF_HDROP - Error: Data not found"), CONSOLE_NEWLINE);
			break;
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Clippiece create description by CF_HDROP - Error: Unable to lock data"), CONSOLE_NEWLINE);
			if (hFileLock)
				GlobalUnlock(hFileContent);
			break;
		case FORMAT_ERROR_NO_FILE:
			consoleOutput(_T("Clippiece create description by CF_HDROP - Error: No file found"), CONSOLE_NEWLINE);
			if (hFileLock)
				GlobalUnlock(hFileContent);
			break;
		}
		return false;
	}
	return false;
}

bool setTextDescription(std::basic_string<wchar_t> &des, std::basic_string<wchar_t> &desFull)
{
	if (des.length() > 0)				// Thẩm mỹ: Thêm một lần xuống dòng để phân cách các lần thêm mô tả
		des.append(_T("\n"));

	if (desFull.length() > 0)
		desFull.append(_T("\n"));

	bool isCF_UNICODETEXTAvailable = true;
	HANDLE textContent = (HANDLE)GetClipboardData(CF_UNICODETEXT);		// Ưu tiên CF_UNICODETEXT hơn CF_TEXT
	if (textContent == NULL) {
		textContent = (HANDLE)GetClipboardData(CF_TEXT);
		isCF_UNICODETEXTAvailable = false;
		
		consoleOutput(_T("CF_UNICODETEXT format not found, fall back to CF_TEXT"), CONSOLE_NEWLINE);
	}

	char *buffer = NULL;
	wchar_t *wbuffer = NULL;
	try {
		if (textContent == NULL)
			throw CLIPBOARD_ERROR_DATA_NOT_FOUND;

		if (isCF_UNICODETEXTAvailable) {
			wbuffer = (wchar_t*)GlobalLock(textContent);
			if (wbuffer == NULL)
				throw GLOBALLOCK_FAILED;

			std::basic_string<wchar_t> tempWString(wbuffer);

			des.append(tempWString);
			desFull.append(tempWString);
		}
		else {
			buffer = (char*)GlobalLock(textContent);
			if (buffer == NULL)
				throw GLOBALLOCK_FAILED;

			std::basic_string<char> tempString(buffer);
			std::basic_string<wchar_t> tempWString;
			tempWString.resize(tempString.length());
			std::copy(tempString.begin(), tempString.end(), tempWString.begin());	// Chuyển xâu từ char thành wchar
			
			des.append(tempWString);
			desFull.append(tempWString);
		}

		GlobalUnlock(textContent);

		consoleOutput(_T("Description by CF_TEXT sucessfully created"), CONSOLE_NEWLINE);
		consoleOutput(des.c_str(), CONSOLE_NEWLINE);
		consoleOutput(desFull.c_str(), CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_DATA_NOT_FOUND:
			consoleOutput(_T("Clippiece create description by CF_TEXT - Error: Data not found"), CONSOLE_NEWLINE);
			break;
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Clippiece create description by CF_TEXT - Error: Unable to lock data"), CONSOLE_NEWLINE);
			if (buffer || wbuffer)
				GlobalUnlock(textContent);
			break;
		}
		return false;
	}
	return false;
}

bool setNoneDescription(std::basic_string<wchar_t> &des, std::basic_string<wchar_t> &desFull)
{
	if (des.length() == 0)				// Khi tất cả các hàm tạo mô tả khác đều thất bại
		des.append(_T("Không có bản xem trước"));
	if (desFull.length() == 0)
		desFull.append(_T("Không có bản xem trước đầy đủ"));
	return true;
}