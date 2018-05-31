#pragma once
#include "stdafx.h"

const int MAX_BUFFER_LEN = 1025;

class Client {
private:
	int totalByte;
	int sentByte;
	char* streamBuffer;
	DWORD streamLength;	// Độ dài của dữ liệu
	SOCKET socket;
	Client *next;

public:
	Client();
	~Client();

	void setSocket(SOCKET socket);
	SOCKET getSocket();

	void setTotalByte(int size);	// Thiết lập số byte nhận
	int getTotalByte();				// Lấy tổng số byte nhận

	void setSentByte(int count);	// Thiết lập số byte gửi
	void incrSentByte(int count);	// Tăng số byte đã gửi
	int getSentByte();				// Lấy tổng số byte đã gửi

	void setBufferSize(DWORD length);						// Thiết lập độ dài buffer
	int setBufferContent(char* data, DWORD dataLength);		// Thiết lập nội dung buffer
	int setBufferContent(int integer, DWORD dataLength);
	char* getBuffer();				// Lấy nội dung buffer
	void destroyBuffer();			// Hủy buffer


	int getClientMessage();

	Client* getNextClient();
	void setNextClient(Client* client);
};