// stdafx.h : Header tiền xử lý, giúp tăng tốc độ biên dịch

#pragma once
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Bỏ qua các hàm hiếm dùng
// Thư viện hàm windows
#include <windows.h>
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

// Hằng số

// Chế độ in trong chế độ debug
const int CONSOLE_NORMAL	= 0;
const int CONSOLE_NEWLINE	= 1;
const int CONSOLE_SPACE		= 2;

const int FORMAT_ERROR_NOFILE			= 1000;

const int CLIPBOARD_ERROR_NOTOPEN		= 100;
const int CLIPBOARD_ERROR_DATANOTFOUND	= 101;

const int ALLOCATION_FAILED			= 10;
const int YCLIP_ERROR_RECURSIVE		= 11;
const int GLOBALLOCK_FAILED			= 12;

// Chế độ debug bằng console
const int _debug = 1;

// Hàm tự định nghĩa
void consoleOutput(LPCWSTR, int mode = CONSOLE_NORMAL);
void consoleOutput(DWORD, int mode = CONSOLE_NORMAL);
void consoleOutput(std::wstring, int mode = CONSOLE_NORMAL);
void consoleOutputMode(int);
std::wstring concatNumbertoWString(std::wstring, DWORD);