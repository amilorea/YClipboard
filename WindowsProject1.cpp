// WindowsProject1.cpp : Điểm khởi đầu chương trình tại đây

#include "stdafx.h"
#include "WindowsProject1.h"
#include "clipslot.h"
#include "painter.h"
#include "network.h"

const UINT Window::CF_YCLIP = RegisterClipboardFormat(_T("YClipboard"));
const UINT Window::CF_LOGICAL_POSITION = 0x0;
PASTE_STATE Window::pasteState = NOTHING;
HWND Window::globalhWnd = 0;

DWORD Window::resolveButtonType(WORD identifier) {
	if (identifier < MAX_HOTKEY_COUNT)
		return SELECT_SLOT_BUTTON;
	else if (identifier == MAX_HOTKEY_COUNT)
		return CANCEL_CHANGE_SLOT_BUTTON;
	else if (identifier == (0xFF - 1))
		return CONNECT_BUTTON;
	else if (identifier == (0xFF - 2))
		return DISCONNECT_BUTTON;
	else if (identifier == (0xFF - 3))
		return IP_BOX;
	else if (identifier >= 0xFF) {
		identifier = (identifier - 0xFF) % BUTTON_SKIP_RANGE;
		return identifier;
	}
	return 0;
}

DWORD Window::resolveButtonPosition(DWORD identifier, int buttonType) {
	switch (buttonType) {
	case SHARE_BUTTON:
	case DOWNLOAD_BUTTON:
	case REMOVE_BUTTON:
	case CHANGE_SLOT_BUTTON:
		return (identifier - 0xFF) / BUTTON_SKIP_RANGE;
	case SELECT_SLOT_BUTTON:
		return identifier;
	case CANCEL_CHANGE_SLOT_BUTTON:
		return MAX_HOTKEY_COUNT;
	}
	return -1;
}

// Hook phím
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
HHOOK keyboardHook;

#define MAX_LOADSTRING 100

// Biến toàn cục
HINSTANCE hInst;						// Thể hiện của chương trình
wchar_t szTitle[MAX_LOADSTRING];		// Tiêu đề
wchar_t szWindowClass[MAX_LOADSTRING];	// Cửa sổ chính

// ===== Chuyển tiếp các định nghĩa của các hàm =====
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// Khởi tạo các chuỗi chính
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Khởi tạo thể hiện của chương trình tại đây
    if (!InitInstance (hInstance, nCmdShow)){
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    // Vòng lặp chính của chương trình dùng xử lý các thông điệp
    while (GetMessage(&msg, NULL, 0, 0)){
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

// ===== Đăng ký cửa sổ hiện hành =====
ATOM MyRegisterClass(HINSTANCE hInstance){
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// ===== Đây là hàm khởi tạo thực thể của chương trình và tạo ra các cửa sổ =====
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	hInst = hInstance;

	HWND hWnd = CreateWindowEx(
		0,
		szWindowClass,
		L"YClipboard",
		WS_OVERLAPPEDWINDOW | WS_VSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		Painter::defaultWidth,
		Painter::defaultHeight,
		(HWND)NULL,
		(HMENU)NULL,
		hInstance,
		(PVOID)NULL);
	Window::globalhWnd = hWnd;

	// Khởi tạo các nút bấm
	_painter.addSlotButtonList();
	_painter.addSocketConnector();

	// Khởi tạo mạn
	_network.init();

	if (!hWnd)
		return FALSE;

	// Hiển thị cửa sổ
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

// ===== Đây là hàm mà ta xử lý các phím chức năng trên cửa sổ liên quan tới clippiece =====
void buttonCommandResolve(DWORD buttonType, DWORD clippiecePosition) {
	switch (buttonType) {
	case SHARE_BUTTON: {
		// ==== Chuyển chế độ chia sẻ ====
		consoleOutput(_T("Request toggle sharing for slot"), CONSOLE_SPACE);
		consoleOutput(clippiecePosition, CONSOLE_NEWLINE);

		_clipslot.toggleShareSlot(clippiecePosition);
	} break;
	case REMOVE_BUTTON: {
		// ==== Xóa clippiece ====
		consoleOutput(_T("Remove button pressed for slot"), CONSOLE_SPACE);
		consoleOutput(clippiecePosition, CONSOLE_NEWLINE);

		_clipslot.removeClippiece(clippiecePosition);

		_painter.setVerticalScroll(PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	} break;
	case CHANGE_SLOT_BUTTON: {
		// ==== Yêu cầu đổi phím tắt ====
		consoleOutput(_T("Request change slot button pressed for slot"), CONSOLE_SPACE);
		consoleOutput(clippiecePosition, CONSOLE_NEWLINE);

		if (_clipslot.requestChangeClippiecePosition(clippiecePosition))
			_painter.setPaintMode(PAINTER_MODE_CHANGESLOT);

		_painter.setVerticalScroll(PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	} break;
	case SELECT_SLOT_BUTTON: {
		// ==== Chọn phím tắt ====
		consoleOutput(_T("Select slot "), CONSOLE_SPACE);
		consoleOutput(clippiecePosition, CONSOLE_NEWLINE);

		_clipslot.changeClippiecePosition(clippiecePosition);
		_painter.setPaintMode(PAINTER_MODE_NORMAL);

		_painter.setVerticalScroll(PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	} break;
	case CANCEL_CHANGE_SLOT_BUTTON: {
		// ==== Hủy yêu cầu đổi phím tắt ====
		consoleOutput(_T("Cancel hotkey change for slot"), CONSOLE_SPACE);
		consoleOutput(clippiecePosition, CONSOLE_NEWLINE);

		_clipslot.cancelChangeClippiecePosition(clippiecePosition);
		_painter.setPaintMode(PAINTER_MODE_NORMAL);
		_painter.redraw();
	} break;
	case CONNECT_BUTTON: {
		// ==== Kết nối ====
		consoleOutput(_T("Connection request"), CONSOLE_NEWLINE);

		if (_network.switchConnect())
			if (!_network.getClippieceDescription())
				_network.switchConnect();
		_painter.addDownloadButton();
		_painter.setVerticalScroll(PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	} break;
	case DISCONNECT_BUTTON: {
		// ==== Ngắt kết nối ====
		consoleOutput(_T("Disconnection request"), CONSOLE_NEWLINE);

		_network.switchConnect();
		_clipslot.cleanSharedData();
		_painter.setVerticalScroll(PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
	} break;
	case DOWNLOAD_BUTTON: {
		// ==== Tải về ====
		consoleOutput(_T("Download request for slot"), CONSOLE_SPACE);
		consoleOutput(clippiecePosition, CONSOLE_NEWLINE);

		DWORD logicalPosition = _clipslot.getLogicalPositionFromSharedClippiece(clippiecePosition);
		if (logicalPosition != CLIPPIECE_NOSLOT)
			if (_network.downloadToPosition(clippiecePosition, logicalPosition)) {
				_painter.addRemoveButton(clippiecePosition);
				_painter.addSlotButton(clippiecePosition);
				_painter.addShareButton(clippiecePosition);
				_painter.redraw();
			}
	} break;
	}
}

// ===== Đây là hàm mà ta xử lý các thông điệp được gửi tới =====
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	HDC hdc;
	PAINTSTRUCT ps;
    switch (message){
		case WM_CREATE: {
			// ==== Thông điệp khởi tạo cửa sổ chính ====
			consoleOutput(_T("Initializing the main window"), CONSOLE_NEWLINE);
			keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
			consoleOutput(_T("Keyboard hooked"), CONSOLE_NEWLINE);
			AddClipboardFormatListener(hWnd);
			consoleOutput(_T("Listener added"), CONSOLE_NEWLINE);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)Painter::globalFontArialMedium, MAKELPARAM(FALSE, 0));
			consoleOutput(_T("Arial font set"), CONSOLE_NEWLINE);
		} break;
		case WM_MOUSEMOVE: {
			POINTS clickedPoint = MAKEPOINTS(lParam);
			int clickedPosition = _painter.moveEvent(clickedPoint.y, clickedPoint.x);
			if (_clipslot.getClippiece(clickedPosition)) {
				SetCursor(Painter::pointerCursor);
			}
			else SetCursor(Painter::normalCursor);
		} break;
		case WM_LBUTTONDOWN: {
			// ==== Thông điệp nhập từ chuột ====
			if (wParam == MK_LBUTTON) {
				// Khi bấm chuột trái vào ô clippiece, ta sẽ đẩy dữ liệu của nó vào clipboard
				POINTS clickedPoint = MAKEPOINTS(lParam);
				int clickedPosition = _painter.clickEvent(clickedPoint.y, clickedPoint.x);
				Clippiece *triggeredClippiece = _clipslot.triggerClippieceSlot(clickedPosition);

				// Hiển thị thông báo
				std::thread wrapper(&Painter::timedText, &_painter, triggeredClippiece);
				wrapper.detach();
			}
		} break;
		case WM_COMMAND: {
			// ==== Thông điệp nhập từ các thành phần trên cửa sổ ====
            int wmId = LOWORD(wParam);
            switch (wmId) {
            case IDM_EXIT:	// Nút tắt (X)
                DestroyWindow(hWnd);
                break;
            }

			WORD buttonIdentifier = LOWORD(wParam);
			DWORD buttonType = Window::resolveButtonType(buttonIdentifier);
			DWORD clippiecePosition = Window::resolveButtonPosition(buttonIdentifier, buttonType);
			buttonCommandResolve(buttonType, clippiecePosition);
			return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
		case WM_CLIPBOARDUPDATE: {
			// ==== Thông điệp khi nội dung của clipboard thay đổi ====
			DWORD clippieceSlot = _clipslot.createClippieceFromCurrentClipboard();
			if (clippieceSlot != CLIPPIECE_NOSLOT) {
				_painter.addRemoveButton(clippieceSlot);
				_painter.addSlotButton(clippieceSlot);
				_painter.addShareButton(clippieceSlot);
			}

			_painter.setVerticalScroll(PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
		}
		break;
		case WM_GETMINMAXINFO: {
			// ==== Thông điệp khi có yêu cầu thay đổi kích thước cửa sổ ====
			// Kích thước của cửa sổ là luôn cố định
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = Painter::minWindowWidth;
			lpMMI->ptMinTrackSize.y = Painter::minWindowHeight;
			lpMMI->ptMaxTrackSize.x = Painter::maxWindowWidth;
		}
		case WM_PAINT: {
			// ==== Thông điệp khi có yêu cầu vẽ lại cửa sổ ====
            hdc = BeginPaint(hWnd, &ps);

			SelectObject(hdc, Painter::globalFontArial);
			_painter.paint(hdc);

            EndPaint(hWnd, &ps);
        }
        break;
		case WM_VSCROLL: {
			// ==== Thông điệp khi có sự thay đổi thanh cuộn dọc ====
			WORD scrollEvent = LOWORD(wParam);
			_painter.updateVerticalScrollPosition(scrollEvent);
		}
		break;
		case WM_SIZE: {
			// ==== Thông điệp khi có sự thay đổi về kích thước cửa sổ ====
			Painter::windowHeight = HIWORD(lParam);
			Painter::windowWidth = LOWORD(lParam);

			_painter.setVerticalScroll(PAINTER_SCROLL_REMAIN_CURRENT_POSITION);
		}
		break;
		case WM_EXITSIZEMOVE: {
			// ==== Thông điệp sau khi người dùng di chuyển cửa sổ sang vị trí khác ====
			_painter.redraw();
			return 0;
		}
		break;
		case WM_DESTROY: {
			// ==== Thông điệp khi cửa sổ đóng ====
			if(UnhookWindowsHookEx(keyboardHook))
				consoleOutput(_T("Unhooked"), CONSOLE_NEWLINE);
			if(RemoveClipboardFormatListener(hWnd))
				consoleOutput(_T("Format listener removed"), CONSOLE_NEWLINE);
			if(DeleteObject(Painter::globalFontArial))
				consoleOutput(_T("Delete font"), CONSOLE_NEWLINE);
			if(DeleteObject(Painter::globalFontArialMedium))
				consoleOutput(_T("Delete font"), CONSOLE_NEWLINE);
			if (closesocket(Network::masterSocket))
				consoleOutput(_T("Socket closed"), CONSOLE_NEWLINE);
			if (WSACleanup() == 0)
				consoleOutput(_T("WSA cleaned up"), CONSOLE_NEWLINE);
			consoleOutput(_T("Clean and close"), CONSOLE_NEWLINE);
			PostQuitMessage(0);
		}
        break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Xử lý với nút About, ở đây không cần thiết
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Automata để nhận tổ hợp phím
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;
	bool stopPropagation = false;
	if (nCode == HC_ACTION) {
		if (wParam == WM_KEYDOWN) {
			switch (key->vkCode) {
			case VK_RCONTROL:
			case VK_LCONTROL: {
				if (Window::pasteState == NOTHING) {
					Window::pasteState = HOLD_CTRL;
					
					consoleOutput(_T("Key: Hold control"), 1);
				}
			} break;
			case 'V': {
				if (Window::pasteState == HOLD_CTRL) {
					
					consoleOutput(_T("Paste event fired here"), 1);
				}
			} break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0': {
				if (Window::pasteState == HOLD_CTRL) {
					Clippiece* triggeredClippiece = _clipslot.triggerClippieceSlot(key->vkCode - 0x30);

					// Hiển thị thông báo đã sao chép
					std::thread wrapper(&Painter::timedText, &_painter, triggeredClippiece);
					wrapper.detach();
					stopPropagation = true;
					
					consoleOutput(_T("Paste event "), 0);
					consoleOutput(key->vkCode - 0x30, 0);
					consoleOutput(_T(" fired here"), 1);
				}
			} break;
			}
		}
		else if (wParam == WM_KEYUP) {
			switch (key->vkCode) {
			case VK_RCONTROL:
			case VK_LCONTROL: {
				Window::pasteState = NOTHING;
				
				consoleOutput(_T("Key: Release control"), 1);
			} break;
			}
		}
	}

	if (!stopPropagation) {
		return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
	}
	else {
		consoleOutput(_T("Prevent propagation"), 1);
		//return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}