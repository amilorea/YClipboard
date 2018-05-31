#pragma once
#include "stdafx.h"
#include "client.h"

Client::Client() {
	socket = SOCKET_ERROR;
	streamBuffer = NULL;
	totalByte = 0;
	sentByte = 0;
	next = NULL;
	streamLength = 0;
}

Client::~Client() {
	closesocket(socket);
}

// Nguyên tắc đọc: Server chỉ đọc các thông điệp của Client, phần việc hầu hết của Server là gửi trả dữ liệu

// totalByte là tổng số byte sẽ được gửi đi
// phân biệt với streamLength, streamLength và totalByte là như nhau sau khi dữ liệu đã được tạo ra
// vì wrapper là thread chạy song song nên có khả năng totalByte sẽ được đọc trước khi tạo xong dữ liệu
// và gây ra việc truyền dữ liệu sai
void Client::setTotalByte(int n)	{	totalByte = n;		}
int Client::getTotalByte()			{	return totalByte;	}

// sentByte là tổng số byte đã được gửi đi
// khi sentByte bằng totalByte, sự gửi kết thúc
// incrSentByte dùng trong việc truyền dữ liệu nhiều đợt
void Client::setSentByte(int n)		{	sentByte = n;		}
void Client::incrSentByte(int n)	{	sentByte += n;		}
int Client::getSentByte()			{	return sentByte;	}

// Thiết lập socket cho client kết nối đến
void Client::setSocket(SOCKET s)	{	socket = s;		}
SOCKET Client::getSocket()			{	return socket;	}

// Thiết lập buffer
void Client::setBufferSize(DWORD length) {
	if (streamBuffer != NULL) {
		free(streamBuffer);
		streamBuffer = NULL;
	}
	streamBuffer = (char*)malloc(length);
	ZeroMemory(streamBuffer, length);
	streamLength = 0;
}
// Việc này không chỉ ghi dữ liệu mới vào buffer mà còn dịch điểm ghi tới trước để ghi liên tiếp
// LƯU Ý: Độ dài của buffer phải được đặt trước bằng setBufferSize
int Client::setBufferContent(char* data, DWORD dataLength) {
	memcpy(streamBuffer + streamLength, data, dataLength);
	streamLength += dataLength;
	return streamLength;
}
int Client::setBufferContent(int integer, DWORD dataLength) {
	insertDWORD(streamBuffer + streamLength, integer);
	streamLength += dataLength;
	return streamLength;
}

char* Client::getBuffer() {
	return streamBuffer;
}

void Client::destroyBuffer() {
	if (streamBuffer != NULL) {
		free(streamBuffer);
		streamBuffer = NULL;
		streamLength = 0;
	}
}

int Client::getClientMessage() {
	return makeDWORD(streamBuffer[0], streamBuffer[1], streamBuffer[2], streamBuffer[3]);
}

Client* Client::getNextClient()				{	return next;	}
void Client::setNextClient(Client *pNext)	{	next = pNext;	}