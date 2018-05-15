#include "stdafx.h"
#include "clipslot.h"
Clipslot _clipslot;

std::vector<HWND> Clipslot::slotButtonList;

struct sortBySlot
{
	inline bool operator() (Clippiece* cl1, Clippiece* cl2)
	{
		return (cl1->getPosition() < cl2->getPosition());
	}
};

void Clipslot::copyBitmapInfo(PBITMAPINFO source, PBITMAPINFO destination)
{
	destination->bmiHeader.biSize = source->bmiHeader.biSize;
	destination->bmiHeader.biWidth = source->bmiHeader.biWidth;
	destination->bmiHeader.biHeight = source->bmiHeader.biHeight;
	destination->bmiHeader.biPlanes = source->bmiHeader.biPlanes;
	destination->bmiHeader.biBitCount = source->bmiHeader.biBitCount;
	destination->bmiHeader.biCompression = source->bmiHeader.biCompression;
	destination->bmiHeader.biSizeImage = source->bmiHeader.biSizeImage;
	destination->bmiHeader.biXPelsPerMeter = source->bmiHeader.biXPelsPerMeter;
	destination->bmiHeader.biYPelsPerMeter = source->bmiHeader.biYPelsPerMeter;
	destination->bmiHeader.biClrUsed = source->bmiHeader.biClrUsed;
	destination->bmiHeader.biClrImportant = source->bmiHeader.biClrImportant;
}

DWORD Clipslot::colorCalculate(WORD bitCount, DWORD colorUsed, DWORD compression)
{
	DWORD colorCount = 0;
	switch (bitCount) {
	case 0:
		throw CLIPPIECE_ERROR_UNSUPPORTED_FORMAT;
	case 1:
		colorCount = 2;
		break;
	case 4:
	case 8:
		colorCount = colorUsed;
		break;
	case 16:
	case 24:
	case 32:
		if (compression != BI_RGB)
			throw CLIPPIECE_ERROR_UNSUPPORTED_FORMAT;
		colorCount = 0;
		break;
	}
	return colorCount;
}

Clipslot::Clipslot()
{
	defaultSlot = CLIPSLOT_DEFAULT_POSITION;
	clippieceCount = 0;
	hotkeySlotCount = 0;
	requestClippiecePosition = -1;
}

Clipslot::~Clipslot()
{
	consoleOutput(_T("Destroying clipslot"), CONSOLE_NEWLINE);

	std::vector<Clippiece*>::iterator it;
	for (it = clippieceVector.begin(); it != clippieceVector.end(); it++) {
		delete *it;
		*it = NULL;
	}
	clippieceVector.clear();
}

int Clipslot::size()
{
	return clippieceCount;
}

int Clipslot::capacity()
{
	return clippieceVector.size();
}

void Clipslot::setSlotButtonList(HWND button)
{
	slotButtonList.push_back(button);
}

HWND Clipslot::getSlotButton(DWORD selectedButton)
{
	return Clipslot::slotButtonList[selectedButton];
}

Clippiece * Clipslot::getClippiece(DWORD position)
{
	try {
		if (position < 0
		|| position >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		return clippieceVector[position];
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Get clippiece failed - Error: Out of range at position"), CONSOLE_SPACE);
			consoleOutput(position, CONSOLE_NEWLINE);
			break;
		}
		return NULL;
	}
	return NULL;
}

bool Clipslot::setClippiece(Clippiece *addingClippiece, DWORD position)
{
	try {
		// Cẩn thận! Phần tử bị thay thế sẽ KHÔNG bị hủy tự động
		if (position < 0
		|| position >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		clippieceVector[position] = addingClippiece;
		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Set clippiece failed - Error: Out of range at position"), CONSOLE_SPACE);
			consoleOutput(position, CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

Clippiece * Clipslot::getLastClippiece()
{
	return clippieceVector.back();
}

void Clipslot::setLastClippiece(Clippiece* targetClippiece)
{
	// Cẩn thận! Phần tử bị thay thế sẽ KHÔNG bị hủy tự động
	clippieceVector[clippieceVector.size() - 1] = targetClippiece;
}

bool Clipslot::swapClippiece(DWORD position1, DWORD position2) {
	try {
		if (position1 < 0
			|| position1 >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		if (position2 < 0
			|| position2 >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		// Vị trí logic
		if(getClippiece(position1))
			getClippiece(position1)->setPosition(position2);
		if (getClippiece(position2))
			getClippiece(position2)->setPosition(position1);

		// Vị trí vật lý
		Clippiece *temp = clippieceVector[position2];
		clippieceVector[position2] = clippieceVector[position1];
		clippieceVector[position1] = temp;

		consoleOutput(_T("Successfully swapped between 2 positions"), CONSOLE_SPACE);
		consoleOutput(position1, CONSOLE_SPACE);
		consoleOutput(position2, CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Swap clippiece failed - Error: Out of range positions"), CONSOLE_SPACE);
			consoleOutput(position1, CONSOLE_SPACE);
			consoleOutput(position2, CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

bool Clipslot::removeClippiece(DWORD removingPosition)
{
	Clippiece *clippieceToRemove = getClippiece(removingPosition);
	try {
		if (clippieceToRemove == NULL)
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		if (0 <= removingPosition
		&& removingPosition < MAX_HOTKEY_COUNT) {
			if (clippieceToRemove->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
				return true;
			else {
				// Nếu xóa một clippiece đang được gắn hotkey
				clippieceToRemove->setStatus(CLIPPIECE_STATUS_READY_TO_DISCARD);
				// Loại bỏ hoàn toàn dữ liệu của Clippiece này
				clippieceToRemove->destroyData();
				clippieceToRemove->removeAllButton();
				// Việc này thay đổi số lượng phần tử hữu dụng
				clippieceCount -= 1;
				throw CLIPSLOT_DISCARD;
			}
		}

		// Kỹ thuật xóa: Đổi chỗ dữ liệu ở vị trí bị xóa với phần tử cuối cùng của kho chứa
		DWORD lastPosition = capacity() - 1;
		swapClippiece(lastPosition, removingPosition);

		// Việc này thay đổi số lượng phần tử hữu dụng
		clippieceCount -= 1;

		// sau đó loại bỏ phần tử cuối cùng với độ phức tạp O(1)
		delete getLastClippiece();
		clippieceVector.pop_back();

		clippieceToRemove = NULL;

		consoleOutput(_T("Successfully removed clippiece at position"), CONSOLE_SPACE);
		consoleOutput(removingPosition, CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		clippieceToRemove = NULL;
		switch (error) {
		case CLIPSLOT_DISCARD:
			consoleOutput(_T("Remove clippiece - Instead discard clippiece at slot"), CONSOLE_SPACE);
			consoleOutput(removingPosition, CONSOLE_NEWLINE);
			break;
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Remove clippiece failed - Error: No clippiece to remove at slot"), CONSOLE_SPACE);
			consoleOutput(removingPosition, CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

bool Clipslot::requestChangeClippiecePosition(DWORD selectedSlot)
{
	Clippiece *requestClippiece = getClippiece(selectedSlot);
	if (!(requestClippiece == NULL
	|| requestClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)) {
		requestClippiecePosition = selectedSlot;
		return true;
	}
	return false;
}

void Clipslot::cancelChangeClippiecePosition(DWORD selectedSlot)
{
	requestClippiecePosition = -1;
}

DWORD Clipslot::getRequestClippiecePosition()
{
	return requestClippiecePosition;
}

int Clipslot::changeClippiecePosition(DWORD selectedPosition)
{
	try {
		Clippiece *destinationClippiece = getClippiece(selectedPosition);

		if (selectedPosition < 0
		|| selectedPosition >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		consoleOutput(_T("Change position to"), CONSOLE_SPACE);
		consoleOutput(selectedPosition, CONSOLE_NEWLINE);

		if (destinationClippiece == NULL
		|| destinationClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD) {
			// Vị trí được dời tới trống (đã bị discard)
			if (swapClippiece(requestClippiecePosition, selectedPosition)) {
				removeClippiece(requestClippiecePosition);
				int newPosition = addToClipslot(getClippiece(selectedPosition), selectedPosition);
				return newPosition;
			}
			return -1;
		}
		else {
			// Vị trí được dời tới đã bị chiếm
			if (swapClippiece(requestClippiecePosition, selectedPosition)) {
				int newPosition = addToClipslot(getClippiece(selectedPosition), selectedPosition);
				return newPosition;
			}
			return -1;
		}
	}
	catch (int error) {
		switch (error) {
			case CLIPSLOT_ERROR_OUT_OF_RANGE:
				consoleOutput(_T("Clippiece change position failed - Error: Destination out of range at"), CONSOLE_SPACE);
				consoleOutput(selectedPosition, CONSOLE_NEWLINE);
		}
		return -1;
	}
	return -1;
}

bool Clipslot::triggerClippieceSlot(DWORD selectedPosition)
{
	Clippiece *targetClippiece = getClippiece(selectedPosition);
	try {
		if (targetClippiece == NULL
		|| targetClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		consoleOutput(_T("Attempt to trigger clippiece at slot "), CONSOLE_NORMAL);
		consoleOutput(selectedPosition, 1);

		return targetClippiece->injectAll();
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTYSLOT:
			consoleOutput(_T("Clipslot trigger failed - Error: Unoccupied slot"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

int Clipslot::createClippieceFromCurrentClipboard()
{
	/* ATTENTION: Thread? */
	try {
		if (!OpenClipboard(Manager::globalhWnd))
			throw CLIPBOARD_ERROR_NOT_OPEN;

		if (GetClipboardData(Manager::CF_YCLIP) != NULL)
			throw YCLIP_ERROR_RECURSIVE;

		Clippiece *clp = new Clippiece();
		if (clp == NULL)
			throw CLIPPIECE_ERROR_INIT_FAILED;

		HGLOBAL yclipData = GlobalAlloc(GPTR, 1);	// Dữ liệu ở format CF_YCLIP được thêm vào clippiece để tránh việc tạo clippiece đệ quy
		if (yclipData == NULL)
			throw ALLOCATION_FAILED;
		clp->insertData(Manager::CF_YCLIP, yclipData);

		std::basic_string<wchar_t> des;
		std::basic_string<wchar_t> desFull;
		// Tạo ra mục mô tả
		setFileDescription(des, desFull);
		setTextDescription(des, desFull);
		//setImageDescription(des, desFull);
		setNoneDescription(des, desFull);

		UINT uFormat = EnumClipboardFormats(0);		// Lấy format khả dụng đầu tiên trong clipboard
		HANDLE hContent;
		while (uFormat) {
			HGLOBAL hContentCopy = NULL;
			HGLOBAL hDataLock = NULL;
			try {
				//wchar_t szFormatName[100];						// Tên của format
				//
				//consoleOutput(_T("["), 0);
				//consoleOutput(uFormat, 0);
				//consoleOutput(_T("]"), 0);
				//
				//if (_debug) {
				//	consoleOutput(_T("["), 0);
				//	if (GetClipboardFormatName(uFormat, szFormatName, sizeof(szFormatName)))
				//		consoleOutput(szFormatName, 0);
				//	else
				//		consoleOutput(_T("unknown"), 0);
				//	consoleOutput(_T("]: "), 0);
				//}
				hContent = GetClipboardData(uFormat);

				if (hContent) {
					// Chèn dữ liệu
					switch (uFormat) {
					case CF_METAFILEPICT: {

					} break;
					case CF_ENHMETAFILE: {

					} break;
					case CF_DIBV5: {
						PBITMAPV5HEADER bitmapInfo = (PBITMAPV5HEADER)hContent;
						int infoSize = bitmapInfo->bV5Size;
						int infoBitCount = bitmapInfo->bV5BitCount;
						DWORD colorCount = colorCalculate(infoBitCount,
							bitmapInfo->bV5ClrUsed,
							bitmapInfo->bV5Compression);

						DWORD bmSize = bitmapInfo->bV5SizeImage;
						if (bmSize == 0)
							bmSize = bitmapInfo->bV5Height * bitmapInfo->bV5Width * bitmapInfo->bV5BitCount;
						bmSize += sizeof(PBITMAPV5HEADER) + sizeof(RGBQUAD) * colorCount;

						hContentCopy = GlobalAlloc(GPTR, bmSize);
						if (hContentCopy == NULL)
							throw ALLOCATION_FAILED;

						memcpy(hContentCopy, hContent, bmSize);
					} break;
					case CF_DIB: {
						PBITMAPINFO bitmapInfo = (PBITMAPINFO)hContent;
						int infoSize = bitmapInfo->bmiHeader.biSize;
						int infoBitCount = bitmapInfo->bmiHeader.biBitCount;
						DWORD colorCount = colorCalculate(infoBitCount,
							bitmapInfo->bmiHeader.biClrUsed,
							bitmapInfo->bmiHeader.biCompression);

						DWORD bmSize = bitmapInfo->bmiHeader.biSizeImage;
						if (bmSize == 0)
							bmSize = bitmapInfo->bmiHeader.biHeight * bitmapInfo->bmiHeader.biWidth * bitmapInfo->bmiHeader.biBitCount;
						bmSize += sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * colorCount;

						hContentCopy = GlobalAlloc(GPTR, bmSize);
						if (hContentCopy == NULL)
							throw ALLOCATION_FAILED;

						memcpy(hContentCopy, hContent, bmSize);
					} break;
					case CF_BITMAP: {
					} break;
					default: {
						hDataLock = GlobalLock(hContent);

						if (hDataLock == NULL)
							throw GLOBALLOCK_FAILED;

						// Các trường hợp còn lại
						SIZE_T dataSize = GlobalSize(hContent);
						hContentCopy = GlobalAlloc(GPTR, dataSize);
						if (hContentCopy == NULL)
							throw ALLOCATION_FAILED;

						memcpy(hContentCopy, hDataLock, dataSize);
					}
					}

					clp->insertData(uFormat, hContentCopy);
					GlobalUnlock(hContent);
				}
				else {
					consoleOutput(_T("Clipboard change event - Warning: Failed to get clipboard data with format "), 0);
					consoleOutput(uFormat, 1);
				}
			}
			catch (int error) {
				switch (error) {
				case GLOBALLOCK_FAILED:
					consoleOutput(_T("Clipboard change event - Error: Failed to lock data"), CONSOLE_NEWLINE);
					break;
				case ALLOCATION_FAILED:
					consoleOutput(_T("Clipboard change event - Error: Allocation data failed"), CONSOLE_NEWLINE);
					break;
				case CLIPPIECE_ERROR_UNSUPPORTED_FORMAT:
					consoleOutput(_T("Clipboard change event - Error: Unsupported format"), CONSOLE_SPACE);
					consoleOutput(uFormat, CONSOLE_NEWLINE);
					break;
				}

				if (hDataLock)
					GlobalUnlock(hContent);
			}
			uFormat = EnumClipboardFormats(uFormat);	// Lấy format tiếp theo
		}
		CloseClipboard();			// Luôn luôn đóng clipboard

		// Thêm mô tả
		clp->setDes(des);
		clp->setFullDes(desFull);

		return addToClipslot(clp);
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_NOT_OPEN:
			consoleOutput(_T("Clipboard change event - Error: Unable to open clipboard"), CONSOLE_NEWLINE);
			break;
		case ALLOCATION_FAILED:
			consoleOutput(_T("Clipboard change event - Error: Allocation clippiece failed"), CONSOLE_NEWLINE);
			break;
		case YCLIP_ERROR_RECURSIVE:
			consoleOutput(_T("Clipboard change event - Recursive adding prevented"), CONSOLE_NEWLINE);
			break;
		}
		CloseClipboard();
		// To-Do: Destroy the newborn clippiece here
		return CLIPPIECE_POSITION_UNKNOWN;
	}
	return CLIPPIECE_POSITION_UNKNOWN;
}

int Clipslot::addToClipslot(Clippiece *addingClippiece, DWORD position)
{
	try {
		if (addingClippiece->countFormat() <= 1)	// Format duy nhất mà clippiece này có sẽ là format CF_YCLIP
			throw CLIPSLOT_ERROR_EMPTY_CLIPPIECE;

		bool isAffectHotkey = true;
		DWORD selectedPosition = position;

		if (selectedPosition == CLIPSLOT_DEFAULT_POSITION) {
			// Tìm vị trí hotkey còn trống
			for (DWORD hotkeySlot = 0; hotkeySlot < hotkeySlotCount; hotkeySlot++) {
				if (clippieceVector[hotkeySlot] == NULL) {
					selectedPosition = hotkeySlot;
					break;
				}
				if (clippieceVector[hotkeySlot]->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD) {
					selectedPosition = hotkeySlot;
					break;
				}
			}
			// Nếu không tìm thấy mặc định thêm vào cuối dãy
			if (selectedPosition == CLIPSLOT_DEFAULT_POSITION) {
				selectedPosition = clippieceCount;
				if (hotkeySlotCount < MAX_HOTKEY_COUNT) {
					hotkeySlotCount += 1;
					isAffectHotkey = true;
				}
				else
					isAffectHotkey = false;
				clippieceVector.push_back(NULL);
			}
		}

		clippieceCount++;
		if (clippieceVector[selectedPosition] == NULL) {
			// Vị trí thêm vào trống (NULL) thì không xử lý gì thêm
			consoleOutput(_T("Pushed to slot "), CONSOLE_NORMAL);
			consoleOutput(selectedPosition, CONSOLE_NEWLINE);
		}
		else if (clippieceVector[selectedPosition]->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD) {
			// Vị trí thêm vào chứa clippiece được đánh dấu là sẵn sàng để thay thế thì ghi đè an toàn
			consoleOutput(_T("Safety replaced at slot "), CONSOLE_NORMAL);
			consoleOutput(selectedPosition, CONSOLE_NEWLINE);

			delete clippieceVector[selectedPosition];
		}
		else {
			// Trường hợp còn lại: Vị trí thêm vào chứa clippiece vẫn còn hoạt động
			// Trong trường hợp này diễn ra sự thay thế, vậy nên không làm tăng số lượng clippiece
			clippieceCount--;
			
			consoleOutput(_T("Risky replaced at slot "), CONSOLE_NORMAL);
			consoleOutput(selectedPosition, CONSOLE_NEWLINE);
		}

		addingClippiece->setPosition(selectedPosition, isAffectHotkey);
		clippieceVector[selectedPosition] = addingClippiece;

		std::sort(clippieceVector.begin(), clippieceVector.end(), sortBySlot());
		return selectedPosition;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTY_CLIPPIECE:
			delete addingClippiece;
			addingClippiece = NULL;
			consoleOutput(_T("Clipslot adding failed - Error: No data in clippiece"), CONSOLE_NEWLINE);
			break;
		case CLIPSLOT_ERROR_OCCUPIEDSLOT:
			consoleOutput(_T("Clipslot adding failed - Error: The position is occupied"), CONSOLE_NEWLINE);
			break;
		}
		return CLIPPIECE_POSITION_UNKNOWN;
	}
	return CLIPPIECE_POSITION_UNKNOWN;
}
