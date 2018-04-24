// stdafx.cpp : source file that includes just the standard includes
// WindowsProject1.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void consoleOutput(LPCWSTR str, int mode)
{
	OutputDebugString(str);
	consoleOutputMode(mode);
}

void consoleOutput(int d, int mode)
{
	WCHAR buffer[100];					// WCHAR tương đương wchar_t : một ký tự chứa trong 2 byte
	swprintf_s(buffer, 10, L"%d", d);	// swprintf_s tương đương sprintf_s khi dùng cho wchar_t
										// Tiền tố L"" dùng cho wchar_t
	OutputDebugString(buffer);
	consoleOutputMode(mode);
}

void consoleOutput(std::wstring str, int mode)
{
	consoleOutput(str.c_str(), mode);
}

void consoleOutputMode(int mode)
{
	switch (mode) {
	case CONSOLE_NEWLINE:
		OutputDebugString(_T("\n"));
		break;
	case CONSOLE_SPACE:
		OutputDebugString(_T(" "));
		break;
	}
}

std::wstring concatNumbertoWString(std::wstring str, DWORD num)
{
	std::stack<int> st;
	while (num > 0) {
		st.push(num % 10);
		num /= 10;
	}
	while (!st.empty()) {
		str += (char)(st.top() + '0');
		st.pop();
	}
	return str;
}
