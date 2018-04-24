// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WindowsProject1.h"
#include "clipslot.h"
#include "painter.h"

//callback function for the keyboard hook
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

//the hook
HHOOK keyboardHook;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
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

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)){
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)){
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
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

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowEx(
	   0,
	   szWindowClass,
	   szTitle,
	   WS_OVERLAPPEDWINDOW | WS_VSCROLL,
	   CW_USEDEFAULT,
	   CW_USEDEFAULT,
	   Painter::defaultWidth,
	   Painter::defaultHeight,
	   (HWND)NULL,
	   (HMENU)NULL,
	   hInstance,
	   (PVOID)NULL);
   Manager::globalhWnd = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	SCROLLINFO verticalScroll;
	HDC hdc;
	PAINTSTRUCT ps;
    switch (message){
		case WM_CREATE: {
			consoleOutput(_T("Init"), CONSOLE_NEWLINE);
			keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
			AddClipboardFormatListener(hWnd);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)Manager::globalFontArialMedium, MAKELPARAM(FALSE, 0));
		}
		break;
		case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            }

			DWORD buttonType = (LOWORD(wParam) - 0xFF) % 2;
			DWORD triggeredClippieceSlot = (LOWORD(wParam) - 0xFF) / 2;
			switch (buttonType) {
			case REMOVE_BUTTON_LOGICAL: {
				if (_debug) {
					consoleOutput(_T("Remove button pressed for slot"), CONSOLE_SPACE);
					consoleOutput(triggeredClippieceSlot, CONSOLE_NEWLINE);
				}
				_clipVector.removeClippiece(triggeredClippieceSlot);
				//Việc xóa nút gây ra thay đổi về giao diện
				int currentScrollPosition = _painter.getVerticalScrollPosition();
				consoleOutput(_T("Repaint in remove"), CONSOLE_NEWLINE);
				_painter.setVerticalScroll(currentScrollPosition);
			}
			break;
			case SLOT_BUTTON_LOGICAL:
				//_clipVector.changeClippieceSlot(triggeredClippieceSlot);
				//if (_debug) {
				//	consoleOutput(_T("Change slot button pressed for slot"), CONSOLE_SPACE);
				//	consoleOutput(triggeredClippieceSlot, CONSOLE_NEWLINE);
				//}
				break;
			}
			return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
		case WM_CLIPBOARDUPDATE: {
			int clippieceSlot =_clipVector.createClippieceForCurrentClipboard();
			_painter.addRemoveButton(clippieceSlot);
			_painter.addSlotButton(clippieceSlot);

			int currentScrollPosition = _painter.getVerticalScrollPosition();
			consoleOutput(_T("Repaint in update"), CONSOLE_NEWLINE);
			_painter.setVerticalScroll(currentScrollPosition);
		}
		//clipslot.back().display();
		break;
		case WM_GETMINMAXINFO: {
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = 300;
			lpMMI->ptMinTrackSize.y = 200;
		}
		case WM_PAINT: {
            hdc = BeginPaint(hWnd, &ps);

            // TODO: Add any drawing code that uses hdc here...

			SelectObject(hdc, Manager::globalFontArial);
			_painter.paintClippiece(hdc);

            EndPaint(hWnd, &ps);
        }
        break;
		case WM_VSCROLL: {
			WORD scrollEvent = LOWORD(wParam);
			consoleOutput(_T("Repaint in scroll"), CONSOLE_NEWLINE);
			_painter.updateVerticalScrollPosition(scrollEvent);
		}
		break;
		case WM_SIZE: {
			// Lấy kích thước hiện tại của cửa sổ để đặt lại các thuộc tính của con trỏ
			Painter::windowHeight = HIWORD(lParam);
			Painter::windowWidth = LOWORD(lParam);

			int currentScrollPosition = _painter.getVerticalScrollPosition();
			consoleOutput(_T("Repaint in size"), CONSOLE_NEWLINE);
			_painter.setVerticalScroll(currentScrollPosition);
		}
		break;
		case WM_DESTROY: {
			if (_debug) {
				consoleOutput(_T("Clean and close"), CONSOLE_NEWLINE);
			}
			UnhookWindowsHookEx(keyboardHook);
			RemoveClipboardFormatListener(hWnd);
			DeleteObject(Manager::globalFontArial);
			DeleteObject(Manager::globalFontArialMedium);
			PostQuitMessage(0);
		}
        break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;
	bool stopPropagation = false;
	//a key was pressed
	if (nCode == HC_ACTION) {
		if (wParam == WM_KEYDOWN) {
			switch (key->vkCode) {
			case VK_RCONTROL:
			case VK_LCONTROL: {
				if (Manager::pasteState == NOTHING) {
					Manager::pasteState = HOLD_CTRL;
					if (_debug) {
						consoleOutput(_T("Key: Hold control"), 1);
					}
				}
			} break;
			case 'V': {
				if (Manager::pasteState == HOLD_CTRL) {
					if (_debug) {
						consoleOutput(_T("Paste event fired here"), 1);
					}
				}
			} break;
			}
		}
		else if (wParam == WM_SYSKEYDOWN) {
			switch (key->vkCode) {
			case VK_RMENU:
			case VK_LMENU: {
				if (Manager::pasteState == NOTHING) {
					Manager::pasteState = HOLD_ALT;
					if (_debug) {
						consoleOutput(_T("Key: Hold alt"), 1);
					}
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
				if (Manager::pasteState == HOLD_ALT) {
					_clipVector.triggerClippieceSlot(key->vkCode - 0x30);
					stopPropagation = true;
					if (_debug) {
						consoleOutput(_T("Paste event "), 0);
						consoleOutput(key->vkCode - 0x30, 0);
						consoleOutput(_T(" fired here"), 1);
					}
				}
			} break;
			}
		}
		else if (wParam == WM_KEYUP) {
			switch (key->vkCode) {
			case VK_RCONTROL:
			case VK_LCONTROL: {
				Manager::pasteState = NOTHING;
				if (_debug) {
					consoleOutput(_T("Key: Release control"), 1);
				}
			} break;
			}
		}
		else if (wParam == WM_SYSKEYUP) {
			switch (key->vkCode) {
			case VK_RMENU:
			case VK_LMENU: {
				Manager::pasteState = NOTHING;
				stopPropagation = true;
				if (_debug) {
					consoleOutput(_T("Key: Release alt"), 1);
				}
			} break;
			}
		}
	}

	if (!stopPropagation) {
		return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
	}
	else {
		if (_debug) {
			consoleOutput(_T("Prevent propagation"), 1);
		}
		//return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}