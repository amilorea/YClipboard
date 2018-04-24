#include "stdafx.h"
#include "painter.h"

Painter _painter;
int Painter::defaultHeight = 300;
int Painter::defaultWidth = 300;
int Painter::windowHeight = 0;
int Painter::windowWidth = 0;
int Painter::clippieceHeight = 70;
int Painter::clippieceWidth = 260;
int Painter::removeButtonWidth = 80;
int Painter::removeButtonHeight = 20;
int Painter::slotButtonWidth = 45;
int Painter::slotButtonHeight = 20;
int Painter::previewMargin = 5;
int Painter::clippieceMargin = 5;
int Painter::removeButtonMarginTop = 47;
int Painter::removeButtonMarginLeft = 177;
int Painter::slotButtonMarginTop = 3;
int Painter::slotButtonMarginLeft = 212;

Painter::Painter()
{
	lastStartSlot = -1;
	lastEndSlot = -1;
}

bool Painter::paintClippiece(HDC hdc)
{
	consoleOutput(_T("Paint"), CONSOLE_NEWLINE);
	try {
		if (_clipVector.size() == 0)
			throw CLIPSLOT_ERROR_NOCLIPPIECE;

		// Hủy đi vết từ trạng thái trước đó
		for (DWORD i = lastStartSlot; i < lastEndSlot; i++) {
			Clippiece *selectedClippiece = _clipVector.getClippiece(i);
			if (selectedClippiece != NULL) {
				MoveWindow(selectedClippiece->getRemoveButton(),
					-Painter::removeButtonWidth, -Painter::removeButtonHeight,
					Painter::removeButtonWidth, Painter::removeButtonHeight,
					FALSE);	// Không vẽ lại phần cửa sổ bị lộ ra khi di chuyển nút
							// Vì sau này ta vẫn sẽ vẽ đè lên phần không gian đó
				MoveWindow(selectedClippiece->getSlotButton(),
					-Painter::slotButtonWidth, -Painter::slotButtonHeight,
					Painter::slotButtonWidth, Painter::slotButtonHeight,
					FALSE);
			}
		}

		int scrollPosition = getVerticalScrollPosition();
		int newStartSlot = max(
			0,
			scrollPosition / Painter::clippieceHeight
		);
		lastStartSlot = newStartSlot;

		int newEndSlot = min(
			_clipVector.size(),
			(scrollPosition + Painter::windowHeight - 1) / Painter::clippieceHeight + 1
		);
		lastEndSlot = newEndSlot;

		int drawingPoint = (newStartSlot * Painter::clippieceHeight) - scrollPosition;
		DWORD currentSlot = newStartSlot;
		DWORD counter = newStartSlot;
		Clippiece *selectedClippiece;
		while (currentSlot < newEndSlot){
			/*
			┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
			┃ Preview Content      ┏━━━━━━┓ ┃
			┃                      ┃Hotkey┃ ┃
			┃                      ┗━━━━━━┛ ┃
			┃                  ┏━━━━━━━━━━┓ ┃
			┃                  ┃  Remove  ┃ ┃
			┃                  ┗━━━━━━━━━━┛ ┃
			┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
			*/

			/*
				Lưu ý: Hàm vẽ lặp theo số phần tử mà nó phải vẽ vậy nên nếu gặp phải
				phần tử không vẽ được (con trỏ NULL hoặc phần tử bị đánh dấu là đã xóa)
				thì con trỏ vẫn tăng lên trong khi số phần tử vẽ được không đổi
			*/
			selectedClippiece = _clipVector.getClippiece(counter);
			if (selectedClippiece == NULL) {
				counter += 1;
				continue;
			}

			if (selectedClippiece->getStatus() == CLIPPIECE_STATUS_READYTODISCARD) {
				counter += 1;
				continue;
			}

			// Khung chữ nhật của clippiece
			Rectangle(hdc,
				Painter::clippieceMargin, drawingPoint,
				Painter::clippieceWidth + Painter::clippieceMargin, drawingPoint + Painter::clippieceHeight - 1);

			// Bản xem trước
			RECT *previewRectangle = new RECT;
			previewRectangle->top = drawingPoint + Painter::previewMargin;
			previewRectangle->left = Painter::clippieceMargin + Painter::previewMargin;
			previewRectangle->right = Painter::clippieceWidth - Painter::previewMargin;
			previewRectangle->bottom = drawingPoint + Painter::clippieceHeight - Painter::previewMargin;
			DrawText(hdc,
				selectedClippiece->getDes().c_str(),
				-1,
				previewRectangle,
				DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);	// Căn lề trái | Tự gãy dòng | Thêm "..." nếu chưa hết câu
			delete previewRectangle;

			// Nút bấm
			MoveWindow(selectedClippiece->getSlotButton(),
				Painter::clippieceMargin + Painter::slotButtonMarginLeft, Painter::slotButtonMarginTop + drawingPoint,
				Painter::slotButtonWidth, Painter::slotButtonHeight,
				FALSE);
			MoveWindow(selectedClippiece->getRemoveButton(),
				Painter::clippieceMargin + Painter::removeButtonMarginLeft, Painter::removeButtonMarginTop + drawingPoint,
				Painter::removeButtonWidth, Painter::removeButtonHeight,
				FALSE);

			drawingPoint += Painter::clippieceHeight;
			counter += 1;
			currentSlot += 1;
		}
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_NOCLIPPIECE: {
			consoleOutput(_T("Painter clippiece failed - Error: No clippiece to draw"), CONSOLE_NEWLINE);
			return false;
		}
		}
	}

	return true;
}

void Painter::setVerticalScroll(int newPosition)
{
	SCROLLINFO verticalScroll;
	verticalScroll.cbSize = sizeof(verticalScroll);
	verticalScroll.fMask = SIF_ALL;

	verticalScroll.nMin = 0;
	verticalScroll.nMax = Painter::clippieceHeight * _clipVector.size() - Painter::windowHeight + Painter::clippieceHeight;
	verticalScroll.nPage = Painter::clippieceHeight;
	verticalScroll.nPos = newPosition;
	SetScrollInfo(Manager::globalhWnd, SB_VERT, &verticalScroll, TRUE);

	redraw();
}

int Painter::updateVerticalScrollPosition(WORD scrollEvent, int newPosition)
{
	SCROLLINFO verticalScroll;
	verticalScroll.cbSize = sizeof(verticalScroll);
	verticalScroll.fMask = SIF_ALL;
	GetScrollInfo(Manager::globalhWnd, SB_VERT, &verticalScroll);

	int currentVerticalPostion = verticalScroll.nPos;
	int newVerticalPosition = currentVerticalPostion;

	if (newPosition != -1) {
		newVerticalPosition = newPosition;	// Ghi đè các thay đổi sinh ra từ sự kiện
	}
	else {
		switch (scrollEvent) {
		case SB_TOP: {			// Nút HOME
			newVerticalPosition = verticalScroll.nMin;
		} break;
		case SB_BOTTOM: {		// Nút END
			newVerticalPosition = verticalScroll.nMax;
		} break;
		case SB_LINEUP: {		// Nút ↑
			newVerticalPosition -= 15;
		} break;
		case SB_LINEDOWN: {		// Nút ↓
			newVerticalPosition += 15;
		} break;
		case SB_PAGEUP: {		// Nhấn vào khoảng trống phía trên con lăn
			newVerticalPosition -= verticalScroll.nPage;
		} break;
		case SB_PAGEDOWN: {		// Nhấn vào khoảng trống phía dưới con lăn
			newVerticalPosition += verticalScroll.nPage;
		} break;
		case SB_THUMBTRACK: {	// Kéo con lăn bằng tay
			newVerticalPosition = verticalScroll.nTrackPos;
		} break;
		default:
			break;
		}
	}

	// Đặt lại vị trí của con lăn và sau đó lấy vị trí mới của nó
	verticalScroll.fMask = SIF_POS;
	verticalScroll.nPos = newVerticalPosition;
	SetScrollInfo(Manager::globalhWnd, SB_VERT, &verticalScroll, TRUE);

	//if (_debug) {
	//	GetScrollInfo(Manager::globalhWnd, SB_VERT, &verticalScroll);
	//	consoleOutput(_T("New vertical scroll position has been set: "), CONSOLE_NORMAL);
	//	consoleOutput(verticalScroll.nPos, CONSOLE_NEWLINE);
	//}

	redraw();

	return newVerticalPosition;
}

int Painter::getVerticalScrollPosition()
{
	SCROLLINFO verticalScroll;
	verticalScroll.cbSize = sizeof(verticalScroll);
	verticalScroll.fMask = SIF_ALL;
	GetScrollInfo(Manager::globalhWnd, SB_VERT, &verticalScroll);
	return verticalScroll.nPos;
}

bool Painter::addRemoveButton(int selectedSlot)
{
	try {
		Clippiece *selectedClippiece = _clipVector.getClippiece(selectedSlot);
		if (selectedClippiece == NULL)
			throw PAINTER_ERROR_NONEXISTSLOT;

		if (selectedClippiece->getRemoveButton() != NULL)
			throw PAINTER_WARNING_BUTTONEXISTED;

		HWND newRemoveButton = CreateWindowEx(
			0,
			L"BUTTON",  // Predefined class; Unicode assumed
			L"Remove",        // Text will be defined later
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CENTER | BS_PUSHBUTTON | BS_MULTILINE,  // Cho phép tab đến
																							// Hiển thị trong cửa sổ
																							// Là con của cửa sổ chính
																							// Canh giữa
																							// Là một nút bấm
																							// Nội dung có thể xuống dòng
			-Painter::removeButtonWidth,			// x position 
			-Painter::removeButtonHeight,			// y position 
			Painter::removeButtonWidth,		// Button width
			Painter::removeButtonHeight,			// Button height
			Manager::globalhWnd,     // Parent window
			(HMENU)(0xFF + selectedSlot * 2),       // Tăng thêm 0xFF để tránh trùng với các hằng số được định nghĩa trước của window
			(HINSTANCE)GetWindowLong(Manager::globalhWnd, GWL_HINSTANCE),
			NULL);
		SendMessage(newRemoveButton, WM_SETFONT, (WPARAM)Manager::globalFontArial, MAKELPARAM(FALSE, 0));

		selectedClippiece->setRemoveButton(newRemoveButton);

		if (_debug) {
			consoleOutput(_T("Successfully added remove button"), CONSOLE_NEWLINE);
		}

		return true;
	}
	catch (int error) {
		switch (error) {
		case PAINTER_ERROR_NONEXISTSLOT:
			consoleOutput(_T("Painter create remove button failed - Error: No clippiece position exist at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		case PAINTER_WARNING_BUTTONEXISTED:
			consoleOutput(_T("Painter create remove button failed - Warning: Slot button is already exist at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

bool Painter::addSlotButton(int selectedSlot)
{
	try {
		Clippiece *selectedClippiece = _clipVector.getClippiece(selectedSlot);
		if (selectedClippiece == NULL)
			throw PAINTER_ERROR_NONEXISTSLOT;

		if (selectedClippiece->getSlotButton() != NULL)
			throw PAINTER_WARNING_BUTTONEXISTED;

		std::basic_string<WCHAR> buttonText;
		if (selectedSlot >= 0 && selectedSlot <= 9) {
			// Các vị trí từ 0 tới 9 có hotkey mặc định
			buttonText.append(_T("Ctrl "));
			buttonText.push_back(selectedSlot + '0');
		}
		else
		{
			buttonText.append(_T("Hotkey"));
		}

		HWND newSlotButton = CreateWindowEx(
			0,
			L"BUTTON",
			buttonText.c_str(),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CENTER | BS_PUSHBUTTON | BS_MULTILINE,
			-Painter::slotButtonWidth,
			-Painter::slotButtonHeight,
			Painter::slotButtonWidth,
			Painter::slotButtonHeight,
			Manager::globalhWnd,
			(HMENU)(0xFF + selectedSlot * 2 + 1),
			(HINSTANCE)GetWindowLong(Manager::globalhWnd, GWL_HINSTANCE),
			NULL);
		SendMessage(newSlotButton, WM_SETFONT, (WPARAM)Manager::globalFontArial, MAKELPARAM(FALSE, 0));

		selectedClippiece->setSlotButton(newSlotButton);
		consoleOutput(_T("Second:"), CONSOLE_SPACE);
		consoleOutput(selectedClippiece->getDes(), CONSOLE_NEWLINE);

		if (_debug) {
			consoleOutput(_T("Successfully added slot button"), CONSOLE_NEWLINE);
		}

		return true;
	}
	catch (int error) {
		switch (error) {
		case PAINTER_ERROR_NONEXISTSLOT:
			consoleOutput(_T("Painter create slot button failed - Error: No clippiece position exist at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		case PAINTER_WARNING_BUTTONEXISTED:
			consoleOutput(_T("Painter create slot button failed - Warning: Slot button is already exist at slot"), CONSOLE_SPACE);
			consoleOutput(selectedSlot, CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

bool Painter::redraw()
{
	try {
		if (!InvalidateRect(Manager::globalhWnd, NULL, true))
			throw PAINTER_ERROR_FAILTOVALIDATE;

		UpdateWindow(Manager::globalhWnd);
	}
	catch (int error) {
		switch (error) {
		case PAINTER_ERROR_FAILTOVALIDATE:
			consoleOutput(_T("Resize window - Error: Fail to validate client"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return true;
}
