// stdafx.cpp : source file that includes just the standard includes
// WindowsProject1.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void consoleOutput(LPCWSTR str, int mode)
{
	if (_debug) {
		OutputDebugString(str);
		consoleOutputMode(mode);
	}
}

void consoleOutput(int d, int mode)
{
	if (_debug) {
		wchar_t buffer[100];					// WCHAR tương đương wchar_t : một ký tự chứa trong 2 byte
		swprintf_s(buffer, 10, L"%d", d);	// swprintf_s tương đương sprintf_s khi dùng cho wchar_t
											// Tiền tố L"" dùng cho wchar_t
		OutputDebugString(buffer);
		consoleOutputMode(mode);
	}
}

void consoleOutput(std::wstring str, int mode)
{
	if (_debug) {
		consoleOutput(str.c_str(), mode);
	}
}

void consoleOutput(char *str, int mode)
{
	if (_debug) {
		size_t size = strlen(str) + 1;
		size_t returnSize;
		wchar_t buffer[256];
		mbstowcs_s(&returnSize, &buffer[0], 256, str, size);
		OutputDebugString(buffer);
		consoleOutputMode(mode);
	}
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

DWORD bitmapSizeCalculateV5(PBITMAPV5HEADER header)
{
	DWORD result = header->bV5Size + header->bV5SizeImage;
	DWORD colorCount = 0;
	switch (header->bV5BitCount) {
	case 0:
		throw UNSUPPORTED_FORMAT;
	case 1:
		result += sizeof(RGBQUAD) * 2;
		break;
	case 4:
	case 8:
		result += sizeof(RGBQUAD) * header->bV5ClrUsed;
		break;
	case 16:
		if (header->bV5Compression == BI_RGB) break;
		if (header->bV5Compression != BI_BITFIELDS) {
			result += sizeof(DWORD) * 3;
			break;
		}
		throw UNSUPPORTED_FORMAT;
	case 24:
		if (header->bV5Compression == BI_RGB) break;
		if (header->bV5Compression != BI_BITFIELDS) {
			result += sizeof(DWORD) * 3;
			break;
		}
		throw UNSUPPORTED_FORMAT;
	case 32:
		if (header->bV5Compression == BI_RGB) break;
		if (header->bV5Compression != BI_BITFIELDS) {
			result += sizeof(DWORD) * 3;
			break;
		}
		throw UNSUPPORTED_FORMAT;
	}
	return result;
}

DWORD bitmapSizeCalculate(PBITMAPINFO header)
{
	DWORD result = header->bmiHeader.biSizeImage + header->bmiHeader.biSize;
	DWORD colorCount = 0;
	switch (header->bmiHeader.biBitCount) {
	case 0:
		throw UNSUPPORTED_FORMAT;
	case 1:
		result += sizeof(RGBQUAD) * 2;
		break;
	case 4:
	case 8:
		result += sizeof(RGBQUAD) * header->bmiHeader.biClrUsed;
		break;
	case 16:
		if (header->bmiHeader.biCompression == BI_RGB) break;
		if (header->bmiHeader.biCompression != BI_BITFIELDS) {
			result += sizeof(DWORD) * 3;
			break;
		}
		result += sizeof(RGBQUAD) * header->bmiHeader.biClrUsed;
		break;
	case 24:
		if (header->bmiHeader.biCompression == BI_RGB) break;
		result += sizeof(RGBQUAD) * header->bmiHeader.biClrUsed;
		break;
	case 32:
		if (header->bmiHeader.biCompression == BI_RGB) break;
		if (header->bmiHeader.biCompression != BI_BITFIELDS) {
			result += sizeof(DWORD) * 3;
			break;
		}
		result += sizeof(RGBQUAD) * header->bmiHeader.biClrUsed;
		break;
	}
	return result;
}
