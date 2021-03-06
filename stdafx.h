#pragma once
// Macro lặp tuần tự từ 0 đến n - 1
#define FOR(n) for (int cnt = 0; cnt < n; cnt++)
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN		// Bỏ qua các hàm hiếm dùng
// Thư viện hàm windows
#include <windows.h>
#include "Shellapi.h"	// Xử lý kiểu CF_HDROP
#include <stdlib.h>		// Cấp phát bộ nhớ
#include <tchar.h>		// Macro _T
#include <vector>		// Cấu trúc dữ liệu
#include <string>
#include <utility>
#include <map>
#include <algorithm>	// Thuật toán đa dụng
#include <Winsock2.h>	// Socket
#include <Ws2tcpip.h>
#include <thread>		// Thread

// Hằng số
// Chế độ in trong chế độ debug
const int CONSOLE_NORMAL	= 0;
const int CONSOLE_NEWLINE	= 1;	// Xuống dòng sau khi in
const int CONSOLE_SPACE		= 2;	// Thêm một dấu cách sau khi in

const int CLIPBOARD_ERROR_NOT_OPEN			= 0x1F;	// Không mở được clipboard
const int CLIPBOARD_ERROR_DATA_NOT_FOUND	= 0x2F;	// Không có dữ liệu trong clipboard

const int UNKNOWN_ERROR			= 0x0FF;	// Lỗi khác không được quy định
const int ALLOCATION_FAILED		= 0x1FF;	// Không cấp phát được vùng nhớ
const int YCLIP_ERROR_RECURSIVE	= 0x2FF;	// Phát hiện lỗi quay vòng dữ liệu
const int GLOBALLOCK_FAILED		= 0x3FF;	// Thực hiện GlobalLock không thành công
const int HANDLE_FAILURE		= 0x4FF;	// Không lấy được HANDLE
const int UNSUPPORTED_FORMAT	= 0x5FF;	// Kiểu dữ liệu không được hỗ trợ

// Chế độ debug bằng console
const int _debug = 0;

// In ra console
void consoleOutput(LPCWSTR str, int mode = CONSOLE_NORMAL);
void consoleOutput(int num, int mode = CONSOLE_NORMAL);
void consoleOutput(std::wstring str, int mode = CONSOLE_NORMAL);
void consoleOutput(char* data, int mode = CONSOLE_NORMAL);
void consoleOutputMode(int mode);

// Tính toán kích thước bitmap
DWORD bitmapSizeCalculate(PBITMAPINFO header);
DWORD bitmapSizeCalculateV5(PBITMAPV5HEADER header);

// Chèn số vào gói dữ liệu
void insertQWORD(char* buffer, size_t number);
void insertDWORD(char* buffer, DWORD number);
void insertWORD(char* buffer, WORD number);

// Lấy số từ gói dữ liệu
size_t makeQWORD(DWORD high, DWORD low);
size_t makeQWORD(WORD highest, WORD high, WORD low, WORD lowest);
DWORD makeDWORD(char highest, char high, char low, char lowest);
DWORD makeDWORD(WORD high, WORD low);
WORD makeWORD(char high, char low);

// Xem gói dữ liệu dưới dạng byte
void peeking(char* pointer, DWORD size);