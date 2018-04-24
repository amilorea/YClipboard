#include "stdafx.h"
#include "clippiece.h"
#include "control.h"

Clippiece::Clippiece()
{
	slotOccupied = CLIPPIECE_NOSLOT;
	status = CLIPPIECE_STATUS_READYTOUSE;
	removeButton = NULL;
	slotButton = NULL;
}

Clippiece::~Clippiece()
{
	if (_debug) {
		consoleOutput(_T("Destroying clippiece"), CONSOLE_NEWLINE);
	}
	destroyData();
	removeAllButton();
	std::map<UINT, HGLOBAL>::iterator it = dataMap.find(Manager::CF_YCLIP);
	if (it != dataMap.end()) {
		GlobalFree(it->second);
	}
	dataMap.clear();
}

Clippiece::Clippiece(std::basic_string<WCHAR> des)
{
	slotOccupied = CLIPPIECE_NOSLOT;
	setFullDes(des);
	setDes(des);
}

void Clippiece::destroyData() {
	for (std::map<UINT, HGLOBAL>::const_iterator it = dataMap.cbegin(); it != dataMap.cend();) {
		// Phá hủy tất cả dữ liệu trừ CF_YCLIP để chặn việc thêm Clippiece đệ quy
		if (it->second != NULL && it->second != NULL && it->first != Manager::CF_YCLIP) {
			GlobalFree(it->second);
			it = dataMap.erase(it);
		}
		else it++;
	}
	if (_debug) {
		consoleOutput(_T("Successfully destroy data"), CONSOLE_NEWLINE);
	}
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
	if (_debug) {
		consoleOutput(_T("Successfully remove buttons"), CONSOLE_NEWLINE);
	}
}

void Clippiece::setDes(std::basic_string<WCHAR> des)
{
	this->des = des;
	if (_debug) {
		consoleOutput(_T("Des set"), CONSOLE_NEWLINE);
	}
}

std::basic_string<WCHAR> Clippiece::getDes()
{
	return des;
}

void Clippiece::setFullDes(std::basic_string<WCHAR> des)
{
	desFull = des;
	if (_debug) {
		consoleOutput(_T("DesFull set"), CONSOLE_NEWLINE);
	}
}

std::basic_string<WCHAR> Clippiece::getFullDes()
{
	return desFull;
}

void Clippiece::setSlotInfo(int slot)
{
	slotOccupied = slot;

	// Thay đổi nút xóa
	HWND tempRemoveButton = getRemoveButton();
	if (tempRemoveButton != NULL) {
		SetMenu(tempRemoveButton, (HMENU)(0xFF + slot * 2));
	}
	tempRemoveButton = NULL;

	// Thay đổi nút hotkey
	HWND tempSlotButton = getSlotButton();
	if (tempSlotButton != NULL) {
		SetMenu(tempSlotButton, (HMENU)(0xFF + slot * 2 + 1));
		if (0 <= slot && slot <= 9) {
			std::basic_string<WCHAR> buttonText;
			buttonText.append(_T("Ctrl "));
			buttonText.push_back(slot + '0');
			SendMessage(tempSlotButton, WM_SETTEXT, 0, (LPARAM)buttonText.c_str());
		}
	}
	tempSlotButton = NULL;
}

int Clippiece::getSlotInfo()
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
	consoleOutput((int)button, CONSOLE_NEWLINE);
	removeButton = button;
	consoleOutput((int)removeButton, CONSOLE_NEWLINE);
}

HWND Clippiece::getRemoveButton()
{
	return removeButton;
}

void Clippiece::setSlotButton(HWND button)
{
	consoleOutput((int)button, CONSOLE_NEWLINE);
	slotButton = button;
	consoleOutput((int)slotButton, CONSOLE_NEWLINE);
}

HWND Clippiece::getSlotButton()
{
	return slotButton;
}

bool Clippiece::insertData(UINT format, HGLOBAL handle)
{
	try {
		if (handle == NULL)
			throw CLIPPIECE_ERROR_NOHANDLE;
		if (format < 0)
			throw CLIPPIECE_ERROR_NOFORMAT;

		dataMap.insert(std::pair<UINT, HGLOBAL>(format, handle));
		if (_debug) {
			consoleOutput(_T("Clippiece insert succeed with format "), CONSOLE_NORMAL);
			consoleOutput(format, CONSOLE_NEWLINE);
		}
	}
	catch (int error) {
		switch (error) {
		case CLIPPIECE_ERROR_NOHANDLE:
			consoleOutput(_T("Clippiece insert failed - Error: Data handle is NULL "), CONSOLE_NEWLINE);
			return false;
		case CLIPPIECE_ERROR_NOFORMAT:
			consoleOutput(_T("Clippiece insert failed - Error: No such format"), CONSOLE_NEWLINE);
			return false;
		}
	}
	return true;
}

//void Clippiece::display()
//{
//	if (_debug) {
//		consoleOutput(_T("Clippice slot "), 0);
//		consoleOutput(this->slot, 1);
//		consoleOutput(des.c_str(), 1);
//		consoleOutput(desFull.c_str(), 1);
//		WCHAR szFormatName[100];						// Tên của format
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

bool Clippiece::checkFormatAvailable(UINT format)
{
	return dataMap.find(format) != dataMap.end();
}

int Clippiece::getFormatCount()
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
		if (_debug) {
			consoleOutput(_T("Attempt to set clipboard data from slot "), CONSOLE_NORMAL);
			consoleOutput(slotOccupied, 1);
		}

		if (!OpenClipboard(Manager::globalhWnd))
			throw CLIPBOARD_ERROR_NOTOPEN;
		
		EmptyClipboard();		// Gọi hàm Emptyclipboard còn lấy quyền sở hữu clipboard về cho chương trình
		for (std::map<UINT, HGLOBAL>::iterator it = dataMap.begin(); it != dataMap.end(); it++) {
			HGLOBAL hTemp = it->second;
			if (hTemp == NULL) {
				if (_debug){
					consoleOutput(_T("Clippiece inject - Warning: No handle available at format "), CONSOLE_NORMAL);
					consoleOutput(it->first, CONSOLE_NEWLINE);
				}
				continue;
			}

			// Clipboard lấy quyền sở hữu của dữ liệu sau khi nhận và có thể sẽ xóa các dữ liệu này nếu cần
			// Do đó ta không gửi trực tiếp dữ liệu từ clippiece, mà chỉ gửi một bản sao của nó
			SIZE_T dataSize = GlobalSize(hTemp);
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
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_NOTOPEN:
			consoleOutput(_T("Clippiece inject failed - Error: Unable to open clipboard"), CONSOLE_NEWLINE);
			return false;
		}
	}
	return true;
}

bool setFileDescription(std::basic_string<WCHAR> &des, std::basic_string<WCHAR> &desFull)
{
	HANDLE hFileContent = (HANDLE)GetClipboardData(CF_HDROP);
	HDROP hFileLock = NULL;
	UINT fileCount = 0;
	bool firstChance = true;
	try {
		if (hFileContent == NULL)
			throw CLIPBOARD_ERROR_DATANOTFOUND;

		hFileLock = (HDROP)GlobalLock(hFileContent);
		if (hFileLock == NULL)
			throw GLOBALLOCK_FAILED;

		fileCount = DragQueryFile(hFileLock, 0xFFFFFFFF, nullptr, 0);	// Cú pháp đặc trưng của DragQueryFile dùng lấy số tệp

		if (fileCount == 0)
			throw FORMAT_ERROR_NOFILE;

		desFull.append(std::to_wstring(fileCount));
		desFull.append(_T(" tệp / thư mục: "));

		des.append(std::to_wstring(fileCount));
		des.append(_T(" tệp / thư mục:"));

		for (DWORD i = 0; i < fileCount; i++) {
			std::basic_string<WCHAR> fileName;
			UINT fnLength = DragQueryFile(hFileLock, i, nullptr, 0);	// Cú pháp lấy độ dài tên tệp
			fileName.resize(fnLength + 1);								// Tính thêm ký tự \0 kết thúc xâu
			DragQueryFile(hFileLock, i, &(fileName[0]), fnLength + 1);	// Cú pháp ghi lại tên tệp

			desFull.append(_T("\n\t"));
			desFull.append(fileName.c_str());

			if (firstChance) {
				des.append(_T(" "));
				firstChance = false;
			} else des.append(_T(", "));
			des.append(fileName.substr(fileName.find_last_of('\\') + 1).c_str());
		}

		GlobalUnlock(hFileLock);
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_DATANOTFOUND:
			consoleOutput(_T("Clippiece create description by CF_HDROP - Error: Data not found"), CONSOLE_NEWLINE);
			break;
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Clippiece create description by CF_HDROP - Error: Unable to lock data"), CONSOLE_NEWLINE);
			if (hFileLock)
				GlobalUnlock(hFileContent);
			break;
		case FORMAT_ERROR_NOFILE:
			consoleOutput(_T("Clippiece create description by CF_HDROP - Error: No file found"), CONSOLE_NEWLINE);
			if (hFileLock)
				GlobalUnlock(hFileContent);
			break;
		}
		return false;
	}
	if (_debug) {
		consoleOutput(_T("Description sucessfully created"), CONSOLE_NEWLINE);
		consoleOutput(des.c_str(), CONSOLE_NEWLINE);
		consoleOutput(desFull.c_str(), CONSOLE_NEWLINE);
	}
	return true;
}

bool setTextDescription(std::basic_string<WCHAR> &des, std::basic_string<WCHAR> &desFull)
{
	if (des.length() > 0)				// Thẩm mỹ: Thêm một lần xuống dòng để phân cách các lần thêm mô tả
		des.append(_T("\n"));
	if (desFull.length() > 0)
		desFull.append(_T("\n"));

	bool isUsingCF_UNICODETEXT = true;
	HANDLE hFileContent = (HANDLE)GetClipboardData(CF_UNICODETEXT);		// Ưu tiên CF_UNICODETEXT hơn CF_TEXT
	if (hFileContent == NULL) {
		hFileContent = (HANDLE)GetClipboardData(CF_TEXT);
		isUsingCF_UNICODETEXT = false;
		if (_debug) {
			consoleOutput(_T("CF_UNICODETEXT format not found, fall back to CF_TEXT"), CONSOLE_NEWLINE);
		}
	}

	char *buffer = NULL;
	WCHAR *wbuffer = NULL;
	try {
		if (hFileContent == NULL)
			throw CLIPBOARD_ERROR_DATANOTFOUND;

		if (isUsingCF_UNICODETEXT) {
			wbuffer = (WCHAR*)GlobalLock(hFileContent);
			if (wbuffer == NULL)
				throw GLOBALLOCK_FAILED;

			std::basic_string<WCHAR> tempWString(wbuffer);

			des.append(tempWString);
			desFull.append(tempWString);
		}
		else {
			buffer = (char*)GlobalLock(hFileContent);
			if (buffer == NULL)
				throw GLOBALLOCK_FAILED;

			std::basic_string<char> tempString(buffer);
			std::basic_string<WCHAR> tempWString;
			tempWString.resize(tempString.length());
			std::copy(tempString.begin(), tempString.end(), tempWString.begin());	// Chuyển xâu từ char thành wchar

			des.append(tempWString);
			desFull.append(tempWString);
		}

		GlobalUnlock(hFileContent);
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_DATANOTFOUND:
			consoleOutput(_T("Clippiece create description by CF_TEXT - Error: Data not found"), CONSOLE_NEWLINE);
			break;
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Clippiece create description by CF_TEXT - Error: Unable to lock data"), CONSOLE_NEWLINE);
			if (buffer || wbuffer)
				GlobalUnlock(hFileContent);
			break;
		}
		return false;
	}
	if (_debug) {
		consoleOutput(_T("Description sucessfully created"), CONSOLE_NEWLINE);
		consoleOutput(des.c_str(), CONSOLE_NEWLINE);
		consoleOutput(desFull.c_str(), CONSOLE_NEWLINE);
	}
	return true;
}

bool setNoneDescription(std::basic_string<WCHAR> &des, std::basic_string<WCHAR> &desFull)
{
	if (des.length() == 0)				// Khi tất cả các hàm tạo mô tả khác đều thất bại
		des.append(_T("Không có bản xem trước"));
	if (desFull.length() == 0)
		desFull.append(_T("Không có bản xem trước đầy đủ"));
	return false;
}