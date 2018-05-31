#include "stdafx.h"
#include "painter.h"

Painter _painter;
int Painter::minWindowWidth			= 290;
int Painter::maxWindowWidth			= 290;
int Painter::minWindowHeight		= 300;

int Painter::defaultHeight			= 600;
int Painter::defaultWidth			= 300;
int Painter::windowHeight			= 0;
int Painter::windowWidth			= 0;

int Painter::connectorMarginTop		= 5;
int Painter::connectorMarginLeft	= 5;
int Painter::ipBoxHeight			= 20;
int Painter::ipBoxWidth				= 160;
int Painter::connectButtonWidth		= 80;
int Painter::connectButtonHeight	= 20;

int Painter::previewMargin			= 5;
int Painter::clippieceMargin		= 5;
int Painter::clippieceHeight		= 70;
int Painter::clippieceWidth			= 230;

int Painter::removeButtonWidth		= 20;
int Painter::removeButtonHeight		= 20;
int Painter::removeButtonMarginTop	= 3;
int Painter::removeButtonMarginLeft = 220;

int Painter::shareButtonWidth		= 50;
int Painter::shareButtonHeight		= 20;
int Painter::copyButtonMarginTop	= 24;
int Painter::copyButtonMarginLeft	= 200;

int Painter::slotButtonWidth		= 50;
int Painter::slotButtonHeight		= 20;
int Painter::slotButtonMarginTop	= 45;
int Painter::slotButtonMarginLeft	= 200;

int Painter::downloadButtonWidth	= 70;
int Painter::downloadButtonHeight	= 20;
int Painter::downloadButtonMarginTop	= 24;
int Painter::downloadButtonMarginLeft	= 180;

int Painter::changeSlotButtonMarginLeft = 3;
int Painter::changeSlotButtonMarginTop	= 3;

HBRUSH Painter::unusedClippieceColor = CreateSolidBrush(RGB(255, 220, 220));
HFONT Painter::globalFontArial = CreateFont(
	14,
	0,
	0,
	0,
	FW_DONTCARE,
	FALSE,
	FALSE,
	FALSE,
	DEFAULT_CHARSET,
	OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS,
	CLEARTYPE_QUALITY,
	VARIABLE_PITCH,
	TEXT("Arial"));
HFONT Painter::globalFontArialMedium = CreateFont(
	15,
	0,
	0,
	0,
	FW_DONTCARE,
	FALSE,
	FALSE,
	FALSE,
	DEFAULT_CHARSET,
	OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS,
	CLEARTYPE_QUALITY,
	VARIABLE_PITCH,
	TEXT("Arial"));
HCURSOR Painter::normalCursor = LoadCursor(NULL, IDC_ARROW);
HCURSOR Painter::pointerCursor = LoadCursor(NULL, IDC_HAND);

void Painter::timedText(Painter *painter, Clippiece *clp) {
	if (clp != NULL) {
		clp->toggleWait();
		painter->redraw();
		Sleep(1000);
		clp->toggleWait();
		painter->redraw();
	}
}

Painter::Painter() {
	startPosition = -1;
	endPosition = -1;
	paintMode = PAINTER_MODE_NORMAL;
}

Painter::~Painter() {
	consoleOutput(_T("Destroying Painter"), CONSOLE_NEWLINE);
	DeleteObject(Painter::unusedClippieceColor);
}

void Painter::addSlotButtonList() {
	std::vector<HWND> tempList;
	HWND newSlotButton;
	FOR(MAX_HOTKEY_COUNT) {
		std::basic_string<wchar_t> buttonDescription;
		buttonDescription.append(_T("Ctrl "));
		buttonDescription.push_back('0' + cnt);

		newSlotButton = CreateWindowEx(
			0,
			L"BUTTON",
			buttonDescription.c_str(),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CENTER | BS_PUSHBUTTON | BS_MULTILINE,
			// Cho phép tab đến | Hiện | Là cửa sổ con
			// Canh giữa | Là nút bấm | Cho phép nội dung nhiều dòng
			-Painter::slotButtonWidth,
			-Painter::slotButtonHeight,
			Painter::slotButtonWidth,
			Painter::slotButtonHeight,
			Window::globalhWnd,
			(HMENU)cnt,
			(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
			NULL);

		SendMessage(newSlotButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
		_clipslot.setSlotButtonList(newSlotButton);
	}

	HWND newCancelButton = CreateWindowEx(
		0,
		L"BUTTON",
		L"Cancel",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CENTER | BS_PUSHBUTTON | BS_MULTILINE,
		-Painter::slotButtonWidth,
		-Painter::slotButtonHeight,
		Painter::slotButtonWidth,
		Painter::slotButtonHeight,
		Window::globalhWnd,
		(HMENU)MAX_HOTKEY_COUNT,
		(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
		NULL);
	SendMessage(newCancelButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
	_clipslot.setSlotButtonList(newCancelButton);
}
void Painter::addSocketConnector() {
	HWND ipBox = CreateWindowEx(
		0,
		L"EDIT",
		0,
		WS_OVERLAPPED | WS_BORDER | WS_CHILD | WS_VISIBLE,
		Painter::connectorMarginLeft * 2 + Painter::connectButtonWidth, 
		Painter::connectorMarginTop,
		Painter::ipBoxWidth,
		Painter::ipBoxHeight,
		Window::globalhWnd,
		(HMENU)IP_BOX,
		(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
		NULL);
	SendMessage(ipBox, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
	SetWindowPos(ipBox,
		HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE);
	_network.setIpBox(ipBox);

	HWND connectButton = CreateWindowEx(
		0,
		L"BUTTON",
		L"Connect",
		WS_OVERLAPPED | WS_BORDER | WS_CHILD | WS_VISIBLE,
		Painter::connectorMarginLeft,
		Painter::connectorMarginTop,
		Painter::connectButtonWidth,
		Painter::connectButtonHeight,
		Window::globalhWnd,
		(HMENU)CONNECT_BUTTON,
		(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
		NULL);
	SendMessage(connectButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
	SetWindowPos(connectButton,
		HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE);
	_network.setConnectButton(connectButton);

	HWND disconnectButton = CreateWindowEx(
		0,
		L"BUTTON",
		L"Disconnect",
		WS_OVERLAPPED | WS_BORDER | WS_CHILD | WS_VISIBLE,
		Painter::connectorMarginLeft,
		Painter::connectorMarginTop,
		Painter::connectButtonWidth,
		Painter::connectButtonHeight,
		Window::globalhWnd,
		(HMENU)DISCONNECT_BUTTON,
		(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
		NULL);
	SendMessage(disconnectButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
	SetWindowPos(disconnectButton,
		HWND_TOPMOST,
		0, 0, 0, 0,
		NULL);
	ShowWindow(disconnectButton, SW_HIDE);
	EnableWindow(disconnectButton, FALSE);
	_network.setDisconnectButton(disconnectButton);

	consoleOutput(_T("Successfully added ip connector"), CONSOLE_NEWLINE);
}

bool Painter::prepare() {
	try {
		if (_clipslot.size() == 0)
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		// Hủy đi vết từ trạng thái trước đó
		for (DWORD counter = startPosition;
			counter < endPosition;
			counter++) {

			Clippiece *selectedClippiece = _clipslot.getClippiece(counter);
			if (!(selectedClippiece == NULL
			|| selectedClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)) {
				SetWindowPos(selectedClippiece->getRemoveButton(),
					HWND_BOTTOM,
					-Painter::slotButtonWidth, -Painter::slotButtonHeight,
					0, 0,
					SWP_NOSIZE);	// Không vẽ lại phần cửa sổ bị lộ ra khi di chuyển nút
									// Vì sau này ta vẫn sẽ vẽ đè lên phần không gian đó
									// và để tránh gọi hàm vẽ lặp
				SetWindowPos(selectedClippiece->getSlotButton(),
					HWND_BOTTOM,
					-Painter::slotButtonWidth, -Painter::slotButtonHeight,
					0, 0,
					SWP_NOSIZE);
				SetWindowPos(selectedClippiece->getShareButton(),
					HWND_BOTTOM,
					-Painter::shareButtonWidth, -Painter::shareButtonHeight,
					0, 0,
					SWP_NOSIZE);
				SetWindowPos(selectedClippiece->getDownloadButton(),
					HWND_BOTTOM,
					-Painter::downloadButtonWidth, -Painter::downloadButtonHeight,
					0, 0,
					SWP_NOSIZE);
			}
		}

		// Cự ly vẽ
		DWORD scrollPosition = getVerticalScrollPosition();
		startPosition = max(
			0,
			scrollPosition / Painter::clippieceHeight
		);

		endPosition = min(
			_clipslot.capacity(),
			(scrollPosition + Painter::windowHeight - 1) / Painter::clippieceHeight + 1
		);
		drawingPoint = Painter::ipBoxHeight + Painter::connectorMarginTop * 2
			+ (startPosition * Painter::clippieceHeight) - scrollPosition;

		DWORD currentPosition = startPosition;
		DWORD finishPosition = endPosition;
		Clippiece *selectedClippiece;

		// Tọa độ các nút
		int removeButtonX = Painter::clippieceMargin + Painter::removeButtonMarginLeft;
		int removeButtonY = drawingPoint + Painter::removeButtonMarginTop;

		int slotButtonX = Painter::clippieceMargin + Painter::slotButtonMarginLeft;
		int slotButtonY = drawingPoint + Painter::slotButtonMarginTop;

		int copyButtonX = Painter::clippieceMargin + Painter::copyButtonMarginLeft;
		int copyButtonY = drawingPoint + Painter::copyButtonMarginTop;

		int downloadButtonX = Painter::clippieceMargin + Painter::downloadButtonMarginLeft;
		int downloadButtonY = drawingPoint + Painter::downloadButtonMarginTop;

		int accumulateHeight = 0;
		// Tạo ra trạng thái mới
		while (currentPosition < finishPosition) {
			/*
			Lưu ý: Hàm vẽ lặp theo số phần tử mà nó phải vẽ vậy nên nếu gặp phải
			phần tử không vẽ được (con trỏ NULL hoặc phần tử bị đánh dấu là đã xóa)
			thì con trỏ vẫn tăng lên trong khi số phần tử vẽ được không đổi
			*/
			try {
				selectedClippiece = _clipslot.getClippiece(currentPosition);
				if (selectedClippiece == NULL
				|| selectedClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
					throw PAINTER_ERROR_NONEXIST_SLOT;

				switch (paintMode) {
				case PAINTER_MODE_NORMAL: {
					if(selectedClippiece->getSharingStatus() == CLIPPIECE_STATUS_IS_SHARED)
						SetWindowPos(selectedClippiece->getDownloadButton(),
							HWND_BOTTOM,
							downloadButtonX,
							downloadButtonY + accumulateHeight,
							0, 0,
							SWP_NOSIZE);
					else {
						SetWindowPos(selectedClippiece->getSlotButton(),
							HWND_BOTTOM,
							slotButtonX,
							slotButtonY + accumulateHeight,
							0, 0,
							SWP_NOSIZE);
						SetWindowPos(selectedClippiece->getRemoveButton(),
							HWND_BOTTOM,
							removeButtonX,
							removeButtonY + accumulateHeight,
							0, 0,
							SWP_NOSIZE);
						SetWindowPos(selectedClippiece->getShareButton(),
							HWND_BOTTOM,
							copyButtonX,
							copyButtonY + accumulateHeight,
							0, 0,
							SWP_NOSIZE);
					}
				} break;
				case PAINTER_MODE_CHANGESLOT: {
					if (_clipslot.getRequestClippiecePosition() == currentPosition) {
						int changeSlotAccumulateHeight = Painter::changeSlotButtonMarginTop + accumulateHeight;
						int changeSlotAccumulateWidth = Painter::changeSlotButtonMarginLeft;

						int breakPoint = (MAX_HOTKEY_COUNT - 1) / 2;

						for (DWORD selectedSlot = 0; selectedSlot < MAX_HOTKEY_COUNT; selectedSlot++) {
							MoveWindow(_clipslot.getSlotButton(selectedSlot),
								Painter::clippieceMargin + changeSlotAccumulateWidth, drawingPoint + changeSlotAccumulateHeight,
								Painter::slotButtonWidth, Painter::slotButtonHeight,
								FALSE);

							changeSlotAccumulateWidth += (Painter::changeSlotButtonMarginLeft + Painter::slotButtonWidth);

							if (selectedSlot == breakPoint
								|| selectedSlot == MAX_HOTKEY_COUNT - 1) {
								changeSlotAccumulateHeight += (Painter::changeSlotButtonMarginTop + Painter::slotButtonHeight);
								changeSlotAccumulateWidth = Painter::changeSlotButtonMarginLeft;
							}
						}

						// Nút hủy
						MoveWindow(_clipslot.getSlotButton(MAX_HOTKEY_COUNT),
							Painter::clippieceMargin + Painter::changeSlotButtonMarginLeft,
							drawingPoint + changeSlotAccumulateHeight,
							Painter::slotButtonWidth, Painter::slotButtonHeight,
							FALSE);
					}
				}
				}
			}
			catch (int error) {
				switch (error) {
				case PAINTER_ERROR_NONEXIST_SLOT:
					//consoleOutput(_T("Painter preparation - Warning: Non-existed clippiece at slot"), CONSOLE_SPACE);
					//consoleOutput(currentPosition, CONSOLE_NEWLINE);
					break;
				}
			}

			accumulateHeight += Painter::clippieceHeight;
			currentPosition += 1;
		}

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Painter prepare failed - No clippiece to prepare"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
}

bool Painter::paint(HDC hdc) {
	try {
		if (_clipslot.size() == 0)
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		DWORD currentPosition = startPosition;
		DWORD finishPosition = endPosition;
		Clippiece *selectedClippiece;

		int accumulateHeight = drawingPoint;

		while (currentPosition < finishPosition) {
			/*
				Lưu ý: Hàm vẽ lặp theo số phần tử mà nó phải vẽ vậy nên nếu gặp phải
				phần tử không vẽ được (con trỏ NULL hoặc phần tử bị đánh dấu là đã xóa)
				thì con trỏ vẫn tăng lên trong khi số phần tử vẽ được không đổi
			*/
			selectedClippiece = _clipslot.getClippiece(currentPosition);

			// Khung chữ nhật của clippiece
			RECT *outlineRectangle = new RECT;
			outlineRectangle->top = accumulateHeight;
			outlineRectangle->left = Painter::clippieceMargin;
			outlineRectangle->right = Painter::clippieceMargin + Painter::clippieceWidth;
			outlineRectangle->bottom = accumulateHeight + Painter::clippieceHeight - 1;
			Rectangle(hdc,
				outlineRectangle->left, outlineRectangle->top,
				outlineRectangle->right, outlineRectangle->bottom);
			if (selectedClippiece == NULL
			|| selectedClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD) {
				FillRect(hdc, outlineRectangle, Painter::unusedClippieceColor);
			}
			delete outlineRectangle;

			// Bản xem trước
			RECT *previewRectangle = new RECT;
			previewRectangle->top = accumulateHeight + Painter::previewMargin;
			previewRectangle->left = Painter::clippieceMargin + Painter::previewMargin;
			previewRectangle->right = Painter::clippieceWidth - Painter::previewMargin;
			previewRectangle->bottom = accumulateHeight + Painter::clippieceHeight - Painter::previewMargin;
			if (selectedClippiece != NULL) {
				DrawText(hdc,
					selectedClippiece->getDes().c_str(),
					-1,
					previewRectangle,
					DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);	// Căn lề trái | Tự gãy dòng | Thêm "..." nếu chưa hết câu
			}
			else {
				DrawText(hdc,
					L"Dữ liệu trống",
					-1,
					previewRectangle,
					DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);
			}
			delete previewRectangle;

			accumulateHeight += Painter::clippieceHeight;
			currentPosition += 1;
		}

		RECT *outlineRectangle = new RECT;
		outlineRectangle->top = -1;
		outlineRectangle->left = -1;
		outlineRectangle->right = Painter::maxWindowWidth;
		outlineRectangle->bottom = Painter::connectButtonHeight + Painter::connectorMarginTop * 2;
		Rectangle(hdc,
			outlineRectangle->left, outlineRectangle->top,
			outlineRectangle->right, outlineRectangle->bottom);
		delete outlineRectangle;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE: {
			consoleOutput(_T("Painter paint failed - No clippiece to draw"), CONSOLE_NEWLINE);
			break;
		}
		}
		return false;
	}
}

void Painter::setVerticalScroll(int newPosition) {
	if (newPosition == PAINTER_SCROLL_REMAIN_CURRENT_POSITION)
		newPosition = getVerticalScrollPosition();

	SCROLLINFO verticalScroll;
	verticalScroll.cbSize = sizeof(verticalScroll);
	verticalScroll.fMask = SIF_ALL;

	verticalScroll.nMin = 0;
	// Tính toán lại tổng chiều cao thanh cuộn
	verticalScroll.nMax = Painter::ipBoxHeight + Painter::connectorMarginTop * 2
		+ Painter::clippieceHeight * _clipslot.capacity() + Painter::clippieceHeight
		- Painter::windowHeight;
	verticalScroll.nPage = Painter::clippieceHeight;
	verticalScroll.nPos = newPosition;
	SetScrollInfo(Window::globalhWnd, SB_VERT, &verticalScroll, TRUE);

	redraw();
}

int Painter::updateVerticalScrollPosition(WORD scrollEvent, int newPosition) {
	SCROLLINFO verticalScroll;
	verticalScroll.cbSize = sizeof(verticalScroll);
	verticalScroll.fMask = SIF_ALL;
	GetScrollInfo(Window::globalhWnd, SB_VERT, &verticalScroll);

	int currentVerticalPostion = verticalScroll.nPos;
	int newVerticalPosition = currentVerticalPostion;

	if (newPosition != PAINTER_SCROLL_REMAIN_CURRENT_POSITION)
		newVerticalPosition = newPosition;	// Ghi đè các thay đổi sinh ra từ sự kiện
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
	SetScrollInfo(Window::globalhWnd, SB_VERT, &verticalScroll, TRUE);

	redraw();

	return newVerticalPosition;
}

int Painter::getVerticalScrollPosition() {
	SCROLLINFO verticalScroll;
	verticalScroll.cbSize = sizeof(verticalScroll);
	verticalScroll.fMask = SIF_ALL;
	GetScrollInfo(Window::globalhWnd, SB_VERT, &verticalScroll);
	return verticalScroll.nPos;
}

void Painter::setPaintMode(int mode) {
	switch(paintMode) {
	case PAINTER_MODE_CHANGESLOT: {
		for (DWORD selectedSlot = 0; selectedSlot <= MAX_HOTKEY_COUNT; selectedSlot++) {
			MoveWindow(_clipslot.getSlotButton(selectedSlot),
				-Painter::slotButtonWidth, -Painter::slotButtonHeight,
				Painter::slotButtonWidth, Painter::slotButtonHeight,
				FALSE);
		}
	}
	}

	paintMode = mode;
}
int Painter::getPaintMode() {
	return paintMode;
}

bool Painter::addRemoveButton(int selectedSlot) {
	try {
		Clippiece *selectedClippiece = _clipslot.getClippiece(selectedSlot);
		if (selectedClippiece == NULL
		|| selectedClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
			throw PAINTER_ERROR_NONEXIST_SLOT;

		if (selectedClippiece->getRemoveButton() != NULL)
			throw PAINTER_WARNING_BUTTON_EXISTED;

		HWND newRemoveButton = CreateWindowEx(
			0,
			L"BUTTON",  // Predefined class; Unicode assumed
			L"✕",        // Text will be defined later
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
			Window::globalhWnd,     // Parent window
			(HMENU)(0xFF + selectedSlot * BUTTON_SKIP_RANGE + REMOVE_BUTTON),
			(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
			NULL);
		SendMessage(newRemoveButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
		SetWindowPos(newRemoveButton, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

		selectedClippiece->setRemoveButton(newRemoveButton);

		return true;
	}
	catch (int error) {
		switch (error) {
		case PAINTER_ERROR_NONEXIST_SLOT:
			consoleOutput(_T("Painter create remove button failed - No clippiece position exist at slot"), CONSOLE_SPACE);
			break;
		case PAINTER_WARNING_BUTTON_EXISTED:
			consoleOutput(_T("Painter create remove button failed - Warning: Slot button is already exist at slot"), CONSOLE_SPACE);
			break;
		}
		consoleOutput(selectedSlot, CONSOLE_NEWLINE);
		return false;
	}
}
bool Painter::addSlotButton(int selectedSlot) {
	try {
		Clippiece *selectedClippiece = _clipslot.getClippiece(selectedSlot);
		if (selectedClippiece == NULL
		|| selectedClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
			throw PAINTER_ERROR_NONEXIST_SLOT;

		if (selectedClippiece->getSlotButton() != NULL)
			throw PAINTER_WARNING_BUTTON_EXISTED;

		std::basic_string<wchar_t> buttonText;
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
			Window::globalhWnd,
			(HMENU)(0xFF + selectedSlot * BUTTON_SKIP_RANGE + CHANGE_SLOT_BUTTON),
			(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
			NULL);
		SendMessage(newSlotButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
		SetWindowPos(newSlotButton, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

		selectedClippiece->setSlotButton(newSlotButton);

		return true;
	}
	catch (int error) {
		switch (error) {
		case PAINTER_ERROR_NONEXIST_SLOT:
			consoleOutput(_T("Painter create slot button failed - No clippiece position exist at slot"), CONSOLE_SPACE);
			break;
		case PAINTER_WARNING_BUTTON_EXISTED:
			consoleOutput(_T("Painter create slot button failed - Warning: Slot button is already exist at slot"), CONSOLE_SPACE);
			break;
		}
		consoleOutput(selectedSlot, CONSOLE_NEWLINE);
		return false;
	}
}
bool Painter::addShareButton(int selectedSlot) {
	try {
		Clippiece *selectedClippiece = _clipslot.getClippiece(selectedSlot);
		if (selectedClippiece == NULL
		|| selectedClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
			throw PAINTER_ERROR_NONEXIST_SLOT;

		if (selectedClippiece->getShareButton() != NULL)
			throw PAINTER_WARNING_BUTTON_EXISTED;

		HWND newShareButton = CreateWindowEx(
			0,
			L"BUTTON",  // Predefined class; Unicode assumed
			L"Share",        // Text will be defined later
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CENTER | BS_PUSHBUTTON | BS_MULTILINE,  // Cho phép tab đến
																							// Hiển thị trong cửa sổ
																							// Là con của cửa sổ chính
																							// Canh giữa
																							// Là một nút bấm
																							// Nội dung có thể xuống dòng
			-Painter::shareButtonWidth,			// x position 
			-Painter::shareButtonHeight,			// y position 
			Painter::shareButtonWidth,		// Button width
			Painter::shareButtonHeight,			// Button height
			Window::globalhWnd,     // Parent window
			(HMENU)(0xFF + selectedSlot * BUTTON_SKIP_RANGE + SHARE_BUTTON),
			(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
			NULL);
		SendMessage(newShareButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
		SetWindowPos(newShareButton, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

		selectedClippiece->setShareButton(newShareButton);

		return true;
	}
	catch (int error) {
		switch (error) {
		case PAINTER_ERROR_NONEXIST_SLOT:
			consoleOutput(_T("Painter create share button failed - No clippiece position exist at slot"), CONSOLE_SPACE);
			break;
		case PAINTER_WARNING_BUTTON_EXISTED:
			consoleOutput(_T("Painter create share button failed - Warning: Copy button is already exist at slot"), CONSOLE_SPACE);
			break;
		}
		consoleOutput(selectedSlot, CONSOLE_NEWLINE);
		return false;
	}
}
void Painter::addDownloadButton() {
	for (DWORD selectedSlot = 0; selectedSlot < _clipslot.capacity(); selectedSlot++) {
		Clippiece* target = _clipslot.getClippiece(selectedSlot);
		if (target == NULL)
			continue;	// Không tồn tại clippiece

		if (target->getSharingStatus() != CLIPPIECE_STATUS_IS_SHARED)
			continue;	// Clippiece này không phải clippiece được chia sẻ

		if (target->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
			continue;	// Clippiece này đang chờ bị ghi đè

		HWND newDownloadButton = CreateWindowEx(
			0,
			L"BUTTON",
			L"Download",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CENTER | BS_PUSHBUTTON | BS_MULTILINE,
			-Painter::downloadButtonWidth,
			-Painter::downloadButtonHeight,
			Painter::downloadButtonWidth,
			Painter::downloadButtonHeight,
			Window::globalhWnd,
			(HMENU)(0xFF + selectedSlot * BUTTON_SKIP_RANGE + DOWNLOAD_BUTTON),
			(HINSTANCE)GetWindowLongPtr(Window::globalhWnd, GWLP_HINSTANCE),
			NULL);

		SendMessage(newDownloadButton, WM_SETFONT, (WPARAM)Painter::globalFontArial, MAKELPARAM(FALSE, 0));
		target->setDownloadButton(newDownloadButton);
	}
}

// Phân giải vị trí để xác định con trỏ hiển thị
int Painter::clickEvent(WORD ycor, WORD xcor) {
	try {
		if (xcor < Painter::clippieceMargin || xcor >(Painter::clippieceMargin + Painter::clippieceWidth))
			throw PAINTER_WARNING_OUT_OF_X;

		int logicalYCor = ycor - drawingPoint;
		return logicalYCor / Painter::clippieceHeight + startPosition;
	}
	catch (int error) {
		switch (error) {
		case PAINTER_WARNING_OUT_OF_X:
		case PAINTER_WARNING_OUT_OF_Y:
			break;
		}
		return CLIPPIECE_NOSLOT;
	}
}
int Painter::moveEvent(WORD ycor, WORD xcor) {
	try {
		if (xcor < Painter::clippieceMargin || xcor >(Painter::clippieceMargin + Painter::clippieceWidth))
			throw PAINTER_WARNING_OUT_OF_X;

		int logicalYCor = ycor - drawingPoint;
		return logicalYCor / Painter::clippieceHeight + startPosition;
	}
	catch (int error) {
		switch (error) {
		case PAINTER_WARNING_OUT_OF_X:
		case PAINTER_WARNING_OUT_OF_Y:
			break;
		}
		return CLIPPIECE_NOSLOT;
	}
}

bool Painter::redraw()
{
	try {
		prepare();

		if (!InvalidateRect(Window::globalhWnd, NULL, true))
			throw PAINTER_ERROR_FAIL_TO_VALIDATE;

		//UpdateWindow(Window::globalhWnd);
	}
	catch (int error) {
		switch (error) {
		case PAINTER_ERROR_FAIL_TO_VALIDATE:
			consoleOutput(_T("Resize window - Fail to validate client"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return true;
}