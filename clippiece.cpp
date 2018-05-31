#include "stdafx.h"
#include "clippiece.h"

Clippiece::Clippiece() {
	slotOccupied = CLIPPIECE_NOSLOT;
	status = CLIPPIECE_STATUS_READY_TO_USE;
	sharingStatus = CLIPPIECE_STATUS_NO_SHARE;

	removeButton = NULL;
	slotButton = NULL;
	shareButton = NULL;
	downloadButton = NULL;

	uniqueId = -1;
	waiting.append(_T("Đã sao chép"));
}

Clippiece::Clippiece(std::basic_string<wchar_t> description) {
	slotOccupied = CLIPPIECE_NOSLOT;
	status = CLIPPIECE_STATUS_READY_TO_USE;
	sharingStatus = CLIPPIECE_STATUS_NO_SHARE;

	removeButton = NULL;
	slotButton = NULL;
	shareButton = NULL;
	downloadButton = NULL;

	uniqueId = -1;
	waiting.append(_T("Đã sao chép"));
	des.append(description);
}

void Clippiece::toggleWait() {
	// Thay thế mô tả clippiece bằng thông báo đã sao chép
	waiting.swap(des);
}

void Clippiece::setDescription() {
	bool flag = false;
	setTimeDescription();
	if (setFileDescription(flag))	flag = true;
	if (setTextDescription(flag))	flag = true;
	if (setImageDescription(flag))	flag = true;
	if (flag == false)
		setNoneDescription();
}
bool Clippiece::setTimeDescription() {
	SYSTEMTIME time;
	GetLocalTime(&time);
	des.append(std::to_wstring(time.wHour)).append(_T("h /"));
	des.append(std::to_wstring(time.wMinute)).append(_T("m /"));
	des.append(std::to_wstring(time.wSecond)).append(_T(" s - "));
	des.append(std::to_wstring(time.wDay)).append(_T("/"));
	des.append(std::to_wstring(time.wMonth)).append(_T("/"));
	des.append(std::to_wstring(time.wYear));
	return true;
}
bool Clippiece::setFileDescription(bool flag) {
	des.append(_T("\n"));	// Thẩm mỹ: Thêm một lần xuống dòng để phân cách các lần thêm mô tả

	HGLOBAL hFile = NULL;
	HDROP hFileLock = NULL;
	UINT totalFile = 0;

	bool firstOccur = true;
	try {
		std::map<UINT, HGLOBAL>::iterator pos = dataMap.find(CF_HDROP);
		if (pos == dataMap.end())
			throw CLIPPIECE_ERROR_NO_FORMAT;
			

		hFile = pos->second;
		hFileLock = (HDROP)GlobalLock(hFile);
		if (hFileLock == NULL)
			throw GLOBALLOCK_FAILED;

		totalFile = DragQueryFile(hFileLock, 0xFFFFFFFF, nullptr, 0);	// Cú pháp đặc trưng của DragQueryFile dùng lấy số tệp

		if (totalFile == 0)
			throw FORMAT_ERROR_NO_FILE;

		des.append(std::to_wstring(totalFile));
		des.append(_T(" tệp / thư mục:"));

		FOR(totalFile) {
			std::basic_string<wchar_t> fileName;
			UINT fileNameLength = DragQueryFile(hFileLock, cnt, nullptr, 0);	// Cú pháp lấy độ dài tên tệp
			fileName.resize(fileNameLength + 1);								// Tính thêm ký tự \0 kết thúc xâu
			DragQueryFile(hFileLock, cnt, &(fileName[0]), fileNameLength + 1);	// Cú pháp ghi lại tên tệp

			des.append(_T("\n\t"));
			des.append(fileName.c_str());

			if (firstOccur) {
				des.append(_T(" "));
				firstOccur = false;
			}
			else des.append(_T(", "));
			// Bỏ qua đường dẫn phía trước chỉ lấy tên tệp
			des.append(fileName.substr(fileName.find_last_of('\\') + 1).c_str());
		}
		GlobalUnlock(hFileLock);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPPIECE_ERROR_NO_FORMAT:
			consoleOutput(_T("Clippiece create description by CF_HDROP failed - Data not found"), CONSOLE_NEWLINE);
			break;
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Clippiece create description by CF_HDROP failed - Unable to lock data"), CONSOLE_NEWLINE);
			break;
		case FORMAT_ERROR_NO_FILE:
			consoleOutput(_T("Clippiece create description by CF_HDROP faield - No file found"), CONSOLE_NEWLINE);
			break;
		}

		if (hFileLock)
			GlobalUnlock(hFile);
		return false;
	}
}
bool Clippiece::setTextDescription(bool flag) {
	if (flag)				// Thẩm mỹ: Thêm một lần xuống dòng để phân cách các lần thêm mô tả
		des.append(_T("\n"));

	bool isCF_UNICODETEXTAvailable = true;
	HANDLE textContent = NULL;

	char*buffer = NULL;
	wchar_t *wbuffer = NULL;
	try {
		std::map<UINT, HGLOBAL>::iterator pos = dataMap.find(CF_UNICODETEXT);		// Ưu tiên CF_UNICODETEXT hơn CF_TEXT
		if (pos == dataMap.end()) {
			pos = dataMap.find(CF_TEXT);
			isCF_UNICODETEXTAvailable = false;

			consoleOutput(_T("CF_UNICODETEXT format not found, fall back to CF_TEXT"), CONSOLE_NEWLINE);
			if (pos == dataMap.end())
				throw CLIPPIECE_ERROR_NO_FORMAT;
		}

		textContent = pos->second;

		if (isCF_UNICODETEXTAvailable) {
			wbuffer = (wchar_t*)GlobalLock(textContent);
			if (wbuffer == NULL)
				throw GLOBALLOCK_FAILED;

			std::basic_string<wchar_t> tempWString(wbuffer);

			des.append(tempWString);
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
		}

		GlobalUnlock(textContent);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPPIECE_ERROR_NO_FORMAT:
			consoleOutput(_T("Clippiece create description by CF_TEXT - Data not found"), CONSOLE_NEWLINE);
			break;
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Clippiece create description by CF_TEXT - Unable to lock data"), CONSOLE_NEWLINE);
			break;
		}

		if (buffer || wbuffer)
			GlobalUnlock(textContent);
		if (buffer != NULL) delete buffer;
		if (wbuffer != NULL) delete wbuffer;
		return false;
	}
}
bool Clippiece::setImageDescription(bool flag) {
	if (flag)				// Thẩm mỹ: Thêm một lần xuống dòng để phân cách các lần thêm mô tả
		des.append(_T("\n"));

	bool isCF_DIBAvailable = true;
	HANDLE imageContent = NULL;

	try {
		std::map<UINT, HGLOBAL>::iterator pos = dataMap.find(CF_DIB);
		if (pos == dataMap.end()) {
			pos = dataMap.find(CF_DIBV5);
			isCF_DIBAvailable = false;

			consoleOutput(_T("CF_DIB format not found, fall back to CF_DIBV5"), CONSOLE_NEWLINE);
			if (pos == dataMap.end())
				throw CLIPPIECE_ERROR_NO_FORMAT;
		}

		imageContent = pos->second;

		PBITMAPINFO bitmapInfo = (PBITMAPINFO)imageContent;
		des.append(_T("Ảnh: ")).append(std::to_wstring(bitmapInfo->bmiHeader.biWidth))
			.append(_T(" ✕ ")).append(std::to_wstring(bitmapInfo->bmiHeader.biHeight))
			.append(_T("\r\nDung lượng: ")).append(std::to_wstring(bitmapInfo->bmiHeader.biSizeImage / 1024)).append(_T(" KB"));

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPPIECE_ERROR_NO_FORMAT:
			consoleOutput(_T("Clippiece create description by CF_DIB failed - Data not found"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
}

bool Clippiece::setNoneDescription() {
	des.append(_T("Không có bản xem trước"));
	return true;
}

Clippiece::~Clippiece() {
	consoleOutput(_T("Destroying clippiece at position"), CONSOLE_SPACE);
	consoleOutput(getPosition(), CONSOLE_NEWLINE);

	destroyData();
	removeAllButton();
	std::map<UINT, HGLOBAL>::iterator it = dataMap.find(Window::CF_YCLIP);
	if (it != dataMap.end()) GlobalFree(it->second);
	dataMap.clear();
}

void Clippiece::destroyData() {
	des.clear();
	HGLOBAL memorytoDelete = NULL;
	UINT formattoDelete;
	for (std::map<UINT, HGLOBAL>::const_iterator it = dataMap.cbegin(); it != dataMap.cend();) {
		// Phá hủy tất cả dữ liệu trừ CF_YCLIP và CF_LOGICAL_POSITION để chặn việc thêm Clippiece đệ quy
		formattoDelete = it->first;
		memorytoDelete = it->second;
		if (memorytoDelete != NULL &&
			(formattoDelete != Window::CF_YCLIP
			&& formattoDelete != Window::CF_LOGICAL_POSITION)) {	// Các format không cần xóa
			GlobalFree(it->second);
			it = dataMap.erase(it);
		}
		else it++;
	}
}

void Clippiece::removeAllButton() {
	removeRemoveButton();
	removeSlotButton();
	removeShareButton();
	removeDownloadButton();
}

void Clippiece::setStatus(int status)	{	this->status = status;	}
int Clippiece::getStatus()				{	return status;			}

void Clippiece::setSharingStatus(int status)	{	this->sharingStatus = status;	}
int Clippiece::getSharingStatus()				{	return sharingStatus;			}
int Clippiece::toggleSharingStatus() {
	if (sharingStatus == CLIPPIECE_STATUS_IS_SHARING) {
		SetWindowText(shareButton, _T("Share"));
		sharingStatus = CLIPPIECE_STATUS_NO_SHARE;
	}
	else {
		SetWindowText(shareButton, _T("Unshare"));
		sharingStatus = CLIPPIECE_STATUS_IS_SHARING;
	}
	return sharingStatus;
}

void Clippiece::setRemoveButton(HWND button)	{	removeButton = button;	}
HWND Clippiece::getRemoveButton()				{	return removeButton;	}
void Clippiece::removeRemoveButton() {
	if (removeButton != NULL) {
		DestroyWindow(removeButton);
		removeButton = NULL;
	}
}

void Clippiece::setDownloadButton(HWND button)	{	downloadButton = button;	}
HWND Clippiece::getDownloadButton()				{	return downloadButton;	}
void Clippiece::removeDownloadButton() {
	if (downloadButton != NULL) {
		DestroyWindow(downloadButton);
		downloadButton = NULL;
	}
}

void Clippiece::setShareButton(HWND button) {	shareButton = button;		}
HWND Clippiece::getShareButton()			{	return shareButton;		}
void Clippiece::removeShareButton() {
	if (shareButton != NULL) {
		DestroyWindow(shareButton);
		shareButton = NULL;
	}
}

void Clippiece::setSlotButton(HWND button)	{	slotButton = button;	}
HWND Clippiece::getSlotButton()				{	return slotButton;		}
void Clippiece::removeSlotButton() {
	if (slotButton != NULL) {
		DestroyWindow(slotButton);
		slotButton = NULL;
	}
}

void Clippiece::setUniqueId(DWORD id)	{	uniqueId = id;		}
DWORD Clippiece::getUniqueId()			{	return uniqueId;	}

void Clippiece::setDes(std::basic_string<wchar_t> des) {
	des.resize(min(CLIPPIECE_DESCRIPTION_SIZE, des.size()));
	this->des = des;
}
std::basic_string<wchar_t> Clippiece::getDes() {
	return des;
}

void Clippiece::setPosition(DWORD slot, bool isAffectHotkey) {
	slotOccupied = slot;
	// Thay đổi nút xóa
	HWND tempRemoveButton = getRemoveButton();
	if (tempRemoveButton != NULL)
		SetWindowLongPtr(tempRemoveButton, GWLP_ID, (0xFF + slot * BUTTON_SKIP_RANGE + REMOVE_BUTTON));
	// Hàm SetWindowLongPtr dùng thay đổi định danh của nút bấm, dùng cho cửa sổ con

	// Thay đổi nút chuyển hotkey
	HWND tempSlotButton = getSlotButton();
	if (tempSlotButton != NULL) {
		SetWindowLongPtr(tempSlotButton, GWLP_ID, (0xFF + slot * BUTTON_SKIP_RANGE + CHANGE_SLOT_BUTTON));
		std::basic_string<wchar_t> buttonText;
		if (isAffectHotkey) {
			buttonText.append(_T("Ctrl "));
			buttonText.push_back(slot + '0');
		}
		else buttonText.append(_T("Hotkey"));
		SendMessage(tempSlotButton, WM_SETTEXT, 0, (LPARAM)buttonText.c_str());
	}

	// Thay đổi nút chia sẻ
	HWND tempShareButton = getShareButton();
	if (tempShareButton != NULL) 
		SetWindowLongPtr(tempShareButton, GWLP_ID, (0xFF + slot * BUTTON_SKIP_RANGE + SHARE_BUTTON));
	
	consoleOutput(_T("Succesfully set slot to"), CONSOLE_SPACE);
	consoleOutput(getPosition(), CONSOLE_NEWLINE);
}
DWORD Clippiece::getPosition() {
	return slotOccupied;
}

bool Clippiece::insertData(UINT format, HGLOBAL handle) {
	try {
		if (format < 0)
			throw CLIPPIECE_ERROR_NO_FORMAT;

		dataMap.insert(std::pair<UINT, HGLOBAL>(format, handle));

		consoleOutput(_T("Clippiece insert succeed with format "), CONSOLE_NORMAL);
		consoleOutput(format, CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPPIECE_ERROR_NO_FORMAT:
			consoleOutput(_T("Clippiece insert failed - No such format"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
}
bool Clippiece::removeData(UINT format) {
	std::map<UINT, HGLOBAL>::iterator itm = dataMap.find(format);
	if (itm == dataMap.end())
		return false;

	if(format != Window::CF_LOGICAL_POSITION)
		delete itm->second;
	dataMap.erase(itm);

	return true;
}
bool Clippiece::isFormatAvailable(UINT format) {
	return dataMap.find(format) != dataMap.end();
}
int Clippiece::countFormat() {
	return dataMap.size();
}

DWORD Clippiece::getLogicalPosition() {
	std::map<UINT, HGLOBAL>::iterator it = dataMap.find(Window::CF_LOGICAL_POSITION);
	if (it == dataMap.end())
		return CLIPPIECE_NOSLOT;

	return (DWORD)it->second;
}

bool Clippiece::injectAll() {
	try {
		consoleOutput(_T("Attempting to set clipboard data from slot"), CONSOLE_SPACE);
		consoleOutput(getPosition(), CONSOLE_NEWLINE);

		if (!OpenClipboard(Window::globalhWnd))
			throw CLIPBOARD_ERROR_NOT_OPEN;
		
		EmptyClipboard();		// Gọi hàm Emptyclipboard sẽ lấy quyền sở hữu clipboard về cho chương trình

		// Thêm vào format CF_YCLIP để tránh quay vòng
		HGLOBAL cfData = GlobalAlloc(GPTR, 1);
		if (cfData == NULL)
			throw ALLOCATION_FAILED;
		SetClipboardData(Window::CF_YCLIP, cfData);

		for (std::map<UINT, HGLOBAL>::iterator it = dataMap.begin();
			it != dataMap.end();
			it++) {

			UINT format = it->first;
			HGLOBAL data = it->second;

			if (data == NULL) {
				consoleOutput(_T("Clippiece inject - Warning: No handle available at format"), CONSOLE_SPACE);
				consoleOutput(format, CONSOLE_NEWLINE);
				SetClipboardData(format, NULL);
				continue;
			}

			// Clipboard lấy quyền sở hữu của dữ liệu sau khi nhận và có thể sẽ xóa các dữ liệu này nếu cần
			// Do đó ta không gửi trực tiếp dữ liệu từ clippiece, mà chỉ gửi một bản sao của nó
			SIZE_T dataSize = GlobalSize(data);

			consoleOutput(_T("Clippiece inject - Data size:"), CONSOLE_SPACE);
			consoleOutput(dataSize, CONSOLE_NEWLINE);

			HGLOBAL hDataLock = NULL;
			try {
				HGLOBAL hDataCopy = GlobalAlloc(GPTR, dataSize);
				if (hDataCopy == NULL)
					throw ALLOCATION_FAILED;

				hDataLock = GlobalLock(data);
				if (hDataLock == NULL)
					throw GLOBALLOCK_FAILED;

				memcpy(hDataCopy, hDataLock, dataSize);

				GlobalUnlock(data);
				SetClipboardData(format, hDataCopy);
			}
			catch (int error) {
				switch (error) {
				case ALLOCATION_FAILED:
					consoleOutput(_T("Clippiece inject faield - Unable to copy data at format"), CONSOLE_SPACE);
					break;
				case GLOBALLOCK_FAILED:
					consoleOutput(_T("Clippiece inject faield - Unable to lock data at format"), CONSOLE_SPACE);
					break;
				}
				consoleOutput(format, CONSOLE_NEWLINE);

				if (hDataLock)
					GlobalUnlock(data);
			}
		}
		CloseClipboard();

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_NOT_OPEN:
			consoleOutput(_T("Clippiece inject failed - Unable to open clipboard"), CONSOLE_NEWLINE);
		}
		return false;
	}
}

// Cấu trúc: 1: Độ dài của cấu trúc (DWORD) - 2: Vị trí logic (DWORD) - 3: Dữ liệu
char* Clippiece::getSharingDescription(DWORD* size) {
	const wchar_t * convertedDescription = des.c_str();
	*size = sizeof(DWORD) + sizeof(DWORD) + wcslen(convertedDescription) * sizeof(wchar_t);

	char* buffer = (char*)malloc(*size);
	// Phần 1
	insertDWORD(buffer, *size);
	// Phần 2
	insertDWORD(buffer + sizeof(DWORD), slotOccupied);
	// Phần 3
	memcpy(buffer + sizeof(DWORD) + sizeof(DWORD),
		convertedDescription,
		wcslen(convertedDescription) * sizeof(wchar_t));

	return buffer;
}
// Cấu trúc: 1: Tổng độ dài của cấu trúc (DWORD) - 2: Cấu trúc con 1 - ...
// Cấu trúc con: 1.1: Độ dài cấu trúc con - 1.2: Vị trí logic (DWORD) - 1.3: Dữ liệu
char* Clippiece::collectAll(DWORD* totalSize) {
	char* buffer = NULL;
	HGLOBAL hDataLock = NULL;
	try {
		// Phần 1: Tính trước kích thước cần cấp phát
		*totalSize = sizeof(DWORD);
		for (std::map<UINT, HGLOBAL>::iterator mit = dataMap.begin();
			mit != dataMap.end();
			mit++) {
			if (mit->first == Window::CF_YCLIP)
				continue;	// Bỏ qua format CF_YCLIP vì format này khác nhau trên mỗi thiết bị
			*totalSize += sizeof(DWORD);
			*totalSize += sizeof(UINT);
			if(mit->second != NULL)
				*totalSize += GlobalSize(mit->second);
		}

		// Phần 2: Bắt đầu cấp phát và sao chép
		buffer = (char*)malloc(*totalSize);
		if (buffer == NULL)
			throw ALLOCATION_FAILED;

		insertDWORD(buffer, *totalSize);
		DWORD iterator = sizeof(DWORD);
		for (std::map<UINT, HGLOBAL>::iterator mit = dataMap.begin();
			mit != dataMap.end();
			mit++) {
			if (mit->first == Window::CF_YCLIP)
				continue;

			// Tính trước các kích thước cần thiết
			DWORD stepForTotalSize = sizeof(DWORD);
			DWORD stepForFormat = sizeof(UINT);
			DWORD stepForData;
			if (mit->second != NULL)
				stepForData = GlobalSize(mit->second);
			else stepForData = 0;
			DWORD subStructSize = stepForTotalSize + stepForFormat + stepForData;

			// Phần 2.1
			insertDWORD(buffer + iterator, subStructSize);
			iterator += stepForTotalSize;

			// Phần 2.2
			insertDWORD(buffer + iterator, mit->first);
			iterator += stepForFormat;

			// Phần 2.3
			if (mit->second != NULL) {
				hDataLock = GlobalLock(mit->second);
				if (hDataLock == NULL)
					throw GLOBALLOCK_FAILED;
				memcpy(buffer + iterator, hDataLock, stepForData);
				GlobalUnlock(hDataLock);
			}
			iterator += stepForData;
		}

		return buffer;
	}
	catch (int error) {
		switch (error) {
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Collect data failed - Failed to lock data"), CONSOLE_NEWLINE);
			break;
		case ALLOCATION_FAILED:
			consoleOutput(_T("Collect data failed - Cannot allocate memory"), CONSOLE_NEWLINE);
			break;
		}

		if (hDataLock != NULL)
			GlobalUnlock(hDataLock);
		free(buffer);
		*totalSize = 0;
		return NULL;
	}
}
