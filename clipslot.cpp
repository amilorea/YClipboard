#include "stdafx.h"
#include "clipslot.h"

Clipslot _clipVector;
Clipslot::Clipslot()
{
	defaultSlot = CLIPSLOT_DEFAULTSLOT;
	clippieceSlotVector.resize(10);
	for (int i = 0; i < 10; i++)
		clippieceSlotVector[i] = NULL;
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
	int successSlot = -1;
	for (int i = 0; i < CLIPSLOT_MAXSIZE; i++)	// Thử tìm vị trí hotkey còn trống và thêm clippiece này vào nếu có
		if (clippieceSlotVector[i] == NULL) {
			clippieceSlotVector[i] = clippieceVector.back();
			clippieceVector.back()->setSlotInfo(i);
			successSlot = i;
			if (_debug) {
				consoleOutput(_T("Successfully pushed to slot "), CONSOLE_NORMAL);
				consoleOutput(successSlot, CONSOLE_NEWLINE);
			}
			break;
		}

	if (successSlot == -1)
		if (_debug) {
			consoleOutput(_T("Clipslot failed - Warning: No slot available"), CONSOLE_NEWLINE);
		}

	return successSlot;
}

bool Clipslot::triggerClippieceSlot(DWORD selectedSlot)
{
	Clippiece *targetClippiece = clippieceSlotVector[selectedSlot];
	try {
		if (targetClippiece == NULL)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		if (targetClippiece->getFormatCount() == 1)	// Format duy nhất mà clippiece này có là format CF_YCLIP
			throw CLIPSLOT_ERROR_EMPTYCLIPPIECE;

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
		case CLIPSLOT_ERROR_EMPTYCLIPPIECE:
			delete targetClippiece;
			clippieceSlotVector[selectedSlot] = NULL;
			targetClippiece = NULL;
			consoleOutput(_T("Clipslot failed - Error: No data in clippiece"), CONSOLE_NEWLINE);
			break;
		}
	}
	return false;
}

bool Clipslot::createClippieceForCurrentClipboard()
{
	/* ATTENTION: Thread? */
	try {
		if (!OpenClipboard(Manager::globalhWnd))
			throw CLIPBOARD_ERROR_NOTOPEN;

		if (GetClipboardData(Manager::CF_YCLIP) != NULL)
			throw YCLIP_ERROR_RECURSIVE;

		//Clippiece *cl = new Clippiece();
		clippieceVector.push_back(new Clippiece());
		Clippiece *clp = clippieceVector.back();
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

		clp->setDes(des);
		clp->setFullDes(desFull);

		CloseClipboard();			// Luôn luôn đóng clipboard
		addSlot(clp);
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
		return false;
	}
	return true;
}
