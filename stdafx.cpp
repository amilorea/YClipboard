// stdafx.cpp : source file that includes just the standard includes
// WindowsProject1.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

void consoleOutput(LPCWSTR str, int mode) {
	if (_debug) {
		OutputDebugString(str);
		consoleOutputMode(mode);
	}
}
void consoleOutput(int num, int mode) {
	if (_debug) {
		wchar_t buffer[100];
		swprintf_s(buffer, 10, L"%d", num);	// Tiền tố L"" dùng cho wchar_t
		OutputDebugString(buffer);
		consoleOutputMode(mode);
	}
}
void consoleOutput(std::wstring str, int mode) {
	if (_debug) {
		consoleOutput(str.c_str(), mode);
	}
}
void consoleOutput(char*data, int mode) {
	if (_debug) {
		size_t size = strlen(data) + 1;
		size_t returnSize;
		wchar_t buffer[256];
		mbstowcs_s(&returnSize, &buffer[0], 256, data, size);
		OutputDebugString(buffer);
		consoleOutputMode(mode);
	}
}
void consoleOutputMode(int mode) {
	switch (mode) {
	case CONSOLE_NEWLINE:
		OutputDebugString(_T("\n"));
		break;
	case CONSOLE_SPACE:
		OutputDebugString(_T(" "));
		break;
	}
}

DWORD bitmapSizeCalculateV5(PBITMAPV5HEADER header) {
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
DWORD bitmapSizeCalculate(PBITMAPINFO header) {
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

// Việc dịch phải sẽ loại ra số bit tương để khớp với từng ô byte
void insertQWORD(char* buffer, size_t number) {
	FOR(8) buffer[cnt] = number >> (56 - cnt * 8);
}
void insertDWORD(char* buffer, DWORD number) {
	FOR(4) buffer[cnt] = number >> (24 - cnt * 8);
}
void insertWORD(char* buffer, WORD number) {
	FOR(2) buffer[cnt] = number >> (8 - cnt * 8);
}

// Mỗi phần được cho & với mặt nạ để loại bỏ các bit không cần thiết
size_t makeQWORD(DWORD high, DWORD low) {
	size_t result = ((high << 32) & 0xFFFFFFFF00000000) |
							 (low & 0x00000000FFFFFFFF);
	return result;
}
size_t makeQWORD(WORD highest, WORD high, WORD low, WORD lowest)
{
	size_t result = ((highest << 48) & 0xFFFF000000000000) |
					   ((high << 32) & 0x0000FFFF00000000) |
						((low << 16) & 0x00000000FFFF0000) |
							 (lowest & 0x000000000000FFFF);
	return result;
}
DWORD makeDWORD(char highest, char high, char low, char lowest) {
	DWORD result = ((highest << 24) & 0xFF000000) |
					  ((high << 16) & 0x00FF0000) |
						((low << 8) & 0x0000FF00) |
							(lowest & 0x000000FF);
	return result;
}
DWORD makeDWORD(WORD high, WORD low) {
	DWORD result = ((high << 16) & 0xFFFF0000) |
							(low & 0x0000FFFF);
	return result;
}
WORD makeWORD(char high, char low) {
	WORD result = ((high << 8) & 0xFF00) |
						  (low & 0x00FF);
	return result;
}

void peeking(char* pointer, DWORD size) {
	if(!_debug) return;
	FOR(size) consoleOutput((int)pointer[cnt], CONSOLE_SPACE);
}
