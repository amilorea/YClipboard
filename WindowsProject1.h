#pragma once

#include "resource.h"

enum PASTE_STATE { NOTHING, HOLD_CTRL, TYPE_V, HOLD_ALT, TYPE_NUMBER };

const int BUTTON_SKIP_RANGE			= 7;	// Giãn cách định danh giữa các nút

const int SHARE_BUTTON				= 0;	// Nút chia sẻ
const int DOWNLOAD_BUTTON			= 1;	// Nút tải về
const int REMOVE_BUTTON				= 2;	// Nút xóa
const int CHANGE_SLOT_BUTTON		= 3;	// Nút đổi phím tắt
const int SELECT_SLOT_BUTTON		= 4;	// Nút chọn phím tắt
const int CANCEL_CHANGE_SLOT_BUTTON = 5;	// Nút hủy đổi phím tắt

const int BUTTON_PADDING			= 0xFF;	// Giãn cách giữa nút clippiece và nút toàn cục
const int CONNECT_BUTTON			= BUTTON_PADDING - 1;	// Nút kết nối
const int DISCONNECT_BUTTON			= BUTTON_PADDING - 2;	// Nút ngắt kết nối
const int IP_BOX					= BUTTON_PADDING - 3;	// Hộp nhập IP

const int MAX_HOTKEY_COUNT			= 10;	// Số lượng phím tắt tối đa

class Window {
public:
	static const UINT CF_YCLIP;				// Format tự định nghĩa của chương trình
	static const UINT CF_LOGICAL_POSITION;	// Vị trí đặc biệt dùng để lưu vị trí của clippiece
											// trên thiết bị chia sẻ nó
	static PASTE_STATE pasteState;
	static HWND globalhWnd;
	static DWORD resolveButtonType(WORD);
	static DWORD resolveButtonPosition(DWORD, int);
};