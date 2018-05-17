// stdafx.h : Header tiền xử lý, giúp tăng tốc độ biên dịch
#pragma once
#define FOR(n) for (int cnt = 0; cnt < n; cnt++)

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Bỏ qua các hàm hiếm dùng
// Thư viện hàm windows
#include <windows.h>
// Xử lý kiểu CF_HDROP
#include "Shellapi.h"


// Hàm C trong thời gian thực thi
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// Cấu trúc dữ liệu
#include <vector>
#include <cstring>
#include <string>
#include <utility>
#include <map>
#include <stack>

// Thuật toán đa dụng
#include <algorithm>

// Mạng
#include <Winsock2.h>
#include <Ws2tcpip.h>	// InetPton

// Thread
#include <thread>

// Nhập xuất
#include <io.h>

// Hằng số

// Chế độ in trong chế độ debug
const int CONSOLE_NORMAL	= 0;
const int CONSOLE_NEWLINE	= 1;
const int CONSOLE_SPACE		= 2;

const int FORMAT_ERROR_NO_FILE			= 0x01;

const int CLIPBOARD_ERROR_NOT_OPEN			= 0x1F;
const int CLIPBOARD_ERROR_DATA_NOT_FOUND	= 0x2F;

const int UNKNOWN_ERROR = 0x0FF;
const int ALLOCATION_FAILED			= 0x1FF;
const int YCLIP_ERROR_RECURSIVE		= 0x2FF;
const int GLOBALLOCK_FAILED = 0x3FF;
const int HANDLE_FAILURE = 0x4FF;
const int UNSUPPORTED_FORMAT = -0x8FF;

// Chế độ debug bằng console
const int _debug = 1;

// Hàm tự định nghĩa
void consoleOutput(LPCWSTR, int mode = CONSOLE_NORMAL);
void consoleOutput(int, int mode = CONSOLE_NORMAL);
void consoleOutput(std::wstring, int mode = CONSOLE_NORMAL);
void consoleOutput(char*, int mode = CONSOLE_NORMAL);
void consoleOutputMode(int);
std::wstring concatNumbertoWString(std::wstring, DWORD);
DWORD bitmapSizeCalculate(PBITMAPINFO header);
DWORD bitmapSizeCalculateV5(PBITMAPV5HEADER header);