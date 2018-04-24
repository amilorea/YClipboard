#include "stdafx.h"
#include "clipslot.h"

struct sortBySlot
{
	inline bool operator() (Clippiece* cl1, Clippiece* cl2)
	{
		return (cl1->getSlotInfo() < cl2->getSlotInfo());
	}
};

Clipslot _clipVector;
Clipslot::Clipslot()
{
	defaultSlot = CLIPSLOT_DEFAULTSLOT;
	clippieceSlotVector.resize(10);
	clippieceCount = 0;
	for (int i = 0; i < 10; i++)
		clippieceSlotVector.push_back(NULL);
}

Clipslot::~Clipslot()
{
	if (_debug) {
		consoleOutput(_T("Destroying clipslot"), 1);
	}
	std::vector<Clippiece*>::iterator it;
	for (it = clippieceVector.begin(); it != clippieceVector.end(); it++) {
		delete *it;
		*it = NULL;
	}
	for (it = clippieceSlotVector.begin(); it != clippieceSlotVector.end(); it++) {
		*it = NULL;
	}
	clippieceVector.clear();
	clippieceSlotVector.clear();
}

int Clipslot::addSlot(Clippiece *targetClippiece)
{
	try {
		if (targetClippiece->getFormatCount() == 1)	// Format duy nhất mà clippiece này có sẽ là format CF_YCLIP
			throw CLIPSLOT_ERROR_EMPTYCLIPPIECE;

		int lastSlot = clippieceCount;				// Vị trí mặc định của clippiece mới thêm vào sẽ là ở cuối dãy
		int successSlot = -1;

		for (int i = 0; i < CLIPSLOT_MAXHOTKEYCOUNT; i++)	// Thử tìm vị trí hotkey còn trống và thêm clippiece này vào nếu có
			if (clippieceSlotVector[i] == NULL) {
				clippieceSlotVector[i] = targetClippiece;
				successSlot = i;
				targetClippiece->setSlotInfo(successSlot);
				clippieceVector.push_back(NULL);
				clippieceVector[successSlot] = targetClippiece;
				if (_debug) {
					consoleOutput(_T("Successfully pushed to slot "), CONSOLE_NORMAL);
					consoleOutput(successSlot, CONSOLE_NEWLINE);
				}
				break;
			}
			else if (clippieceSlotVector[i]->getStatus() == CLIPPIECE_STATUS_READYTODISCARD) {
				delete clippieceSlotVector[i];
				clippieceSlotVector[i] = NULL;
				clippieceSlotVector[i] = targetClippiece;
				successSlot = i;
				targetClippiece->setSlotInfo(successSlot);
				clippieceVector[successSlot] = targetClippiece;
				if (_debug) {
					consoleOutput(_T("Successfully replaced slot "), CONSOLE_NORMAL);
					consoleOutput(successSlot, CONSOLE_NEWLINE);
				}
				break;
			}

		if (successSlot == -1) {
			// Nếu không thì thêm vào đuôi của mảng
			if (_debug) {
				consoleOutput(_T("Clipslot failed - Warning: No slot available"), CONSOLE_NEWLINE);
			}
			successSlot = lastSlot;
			targetClippiece->setSlotInfo(successSlot);
			clippieceVector.push_back(NULL);
			clippieceVector[successSlot] = targetClippiece;
		}

		std::sort(clippieceVector.begin(), clippieceVector.end(), sortBySlot());
		consoleOutput((int)_clipVector.getClippiece(successSlot)->getRemoveButton(), CONSOLE_NEWLINE);
		consoleOutput((int)_clipVector.getClippiece(successSlot)->getSlotButton(), CONSOLE_NEWLINE);
		clippieceCount++;
		return successSlot;
	}
	catch (int error) {
		switch(error) {
		case CLIPSLOT_ERROR_EMPTYCLIPPIECE:
			delete targetClippiece;
			targetClippiece = NULL;
			consoleOutput(_T("Clipslot failed - Error: No data in clippiece"), CONSOLE_NEWLINE);
			break;
		}
		return -1;
	}
	return -1;
}

Clippiece * Clipslot::getClippiece(DWORD selectedSlot)
{
	try {
		consoleOutput(_T("Attempt to retreive"), CONSOLE_SPACE);
		consoleOutput(selectedSlot, CONSOLE_NEWLINE);
		if (selectedSlot < 0
		|| selectedSlot >= clippieceVector.size())
			throw CLIPSLOT_ERROR_NOCLIPPIECE;

		if (clippieceVector[selectedSlot]->getStatus() == CLIPPIECE_STATUS_READYTODISCARD)
			throw CLIPSLOT_ERROR_EMPTYCLIPPIECE;

		return clippieceVector[selectedSlot];
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_NOCLIPPIECE:
			consoleOutput(_T("Get clippiece failed - Error: No clippiece at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		case CLIPSLOT_ERROR_EMPTYCLIPPIECE:
			consoleOutput(_T("Get clippiece failed - Error: Empty clippiece at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		}
		return NULL;
	}
	return NULL;
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

void Clipslot::removeClippiece(DWORD selectedSlot)
{
	Clippiece *clippieceToRemove = getClippiece(selectedSlot);
	try {
		if (clippieceToRemove == NULL)
			throw CLIPSLOT_ERROR_NOCLIPPIECE;

		if (clippieceToRemove->getStatus() == CLIPPIECE_STATUS_READYTODISCARD)
			throw CLIPSLOT_ERROR_NOCLIPPIECE;

		if (0 <= selectedSlot && selectedSlot <= 9) {
			// Nếu xóa một clippiece đang được gắn hotkey
			clippieceToRemove->setStatus(CLIPPIECE_STATUS_READYTODISCARD);
			// Loại bỏ hoàn toàn dữ liệu của Clippiece này
			clippieceToRemove->destroyData();
			clippieceToRemove->removeAllButton();
			// Việc này thay đổi số lượng phần tử hữu dụng
			clippieceCount -= 1;
			throw CLIPSLOT_DISCARD;
		}

		// Kỹ thuật xóa: Đổi chỗ dữ liệu ở vị trí bị xóa với phần tử cuối cùng của kho chứa
		clippieceVector[selectedSlot] = getLastClippiece();
		setLastClippiece(clippieceToRemove);

		// Việc này thay đổi số lượng phần tử hữu dụng
		clippieceCount -= 1;

		// thay đổi vị trí của phần tử vừa được đem vào thế cho phần tử bị xóa
		clippieceToRemove = clippieceVector[selectedSlot];
		clippieceToRemove->setSlotInfo(selectedSlot);

		// sau đó loại bỏ phần tử cuối cùng với độ phức tạp O(1)
		consoleOutput(_T("Ready to pop"), CONSOLE_NEWLINE);
		clippieceVector.pop_back();

		clippieceToRemove = NULL;
	}
	catch (int error) {
		clippieceToRemove = NULL;
		switch (error) {
		case CLIPSLOT_DISCARD:
			consoleOutput(_T("Remove clippiece - Instead discard clippiece at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		case CLIPSLOT_ERROR_NOCLIPPIECE:
			consoleOutput(_T("Remove clippiece failed - Error: No clippiece to remove at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		}
	}
}

bool Clipslot::triggerClippieceSlot(DWORD selectedSlot)
{
	Clippiece *targetClippiece = getClippiece(selectedSlot);
	try {
		if (targetClippiece == NULL)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		if (_debug) {
			consoleOutput(_T("Now trigger clippiece at slot "), CONSOLE_NORMAL);
			consoleOutput(selectedSlot, 1);
		}
		return targetClippiece->injectAll();
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTYSLOT:
			consoleOutput(_T("Clipslot trigger failed - Error: Unoccupied slot"), CONSOLE_NEWLINE);
			break;
		}
	}
	return false;
}

int Clipslot::createClippieceForCurrentClipboard()
{
	/* ATTENTION: Thread? */
	try {
		if (!OpenClipboard(Manager::globalhWnd))
			throw CLIPBOARD_ERROR_NOTOPEN;

		if (GetClipboardData(Manager::CF_YCLIP) != NULL)
			throw YCLIP_ERROR_RECURSIVE;

		Clippiece *clp = new Clippiece();
		if (clp == NULL)
			throw CLIPPIECE_ERROR_INITFAIL;

		HGLOBAL yclipData = GlobalAlloc(GPTR, 1);	// Dữ liệu ở format CF_YCLIP được thêm vào clippiece để tránh việc tạo clippiece đệ quy
		if (yclipData == NULL)
			throw ALLOCATION_FAILED;
		clp->insertData(Manager::CF_YCLIP, yclipData);

		std::basic_string<WCHAR> des;
		std::basic_string<WCHAR> desFull;
		// Tạo ra mục mô tả
		setFileDescription(des, desFull);
		setTextDescription(des, desFull);
		//setImageDescription(des, desFull);
		setNoneDescription(des, desFull);

		UINT uFormat = EnumClipboardFormats(0);		// Lấy format khả dụng đầu tiên trong clipboard
		HANDLE hContent;
		while (uFormat) {
			//WCHAR szFormatName[100];						// Tên của format
			//if (_debug) {
			//	consoleOutput(_T("["), 0);
			//	consoleOutput(uFormat, 0);
			//	consoleOutput(_T("]"), 0);
			//}
			//if (_debug) {
			//	consoleOutput(_T("["), 0);
			//	if (GetClipboardFormatName(uFormat, szFormatName, sizeof(szFormatName)))
			//		consoleOutput(szFormatName, 0);
			//	else
			//		consoleOutput(_T("unknown"), 0);
			//	consoleOutput(_T("]: "), 0);
			//}
			hContent = GetClipboardData(uFormat);
			HGLOBAL hContentCopy;
			HGLOBAL hDataLock;

			if (hContent) {
				// Chèn dữ liệu
				switch (uFormat) {
				case CF_METAFILEPICT: {

				} break;
				case CF_ENHMETAFILE: {

				} break;
				case CF_DIBV5: {

				} break;
				case CF_DIB: {

				} break;
				case CF_BITMAP: {

				} break;
				default: {
					// Các trường hợp còn lại
					SIZE_T dataSize = GlobalSize(hContent);
					hContentCopy = GlobalAlloc(GPTR, dataSize);
					try {
						hDataLock = GlobalLock(hContent);
						if (hContentCopy == NULL)
							throw ALLOCATION_FAILED;

						if (hDataLock == NULL)
							throw GLOBALLOCK_FAILED;

						memcpy(hContentCopy, hDataLock, dataSize);
						clp->insertData(uFormat, hContentCopy);

						GlobalUnlock(hContent);
					}
					catch (int error) {
						switch (error) {
						case GLOBALLOCK_FAILED:
							consoleOutput(_T("Clipboard change event - Error: Failed to lock data"), CONSOLE_NEWLINE);
							break;
						case ALLOCATION_FAILED:
							consoleOutput(_T("Clipboard change event - Error: Allocation data failed"), CONSOLE_NEWLINE);
							break;
						}

						if (hDataLock)
							GlobalUnlock(hContent);
					}
				}
				}
			}
			else {
				consoleOutput(_T("Clipboard change event - Warning: Failed to get clipboard data with format "), 0);
				consoleOutput(uFormat, 1);
			}

			uFormat = EnumClipboardFormats(uFormat);	// Lấy format tiếp theo
		}
		CloseClipboard();			// Luôn luôn đóng clipboard

		// Thêm mô tả
		clp->setDes(des);
		clp->setFullDes(desFull);

		return addSlot(clp);
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_NOTOPEN:
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
		return -1;
	}
	return -1;
}

int Clipslot::size()
{
	return clippieceCount;
}
