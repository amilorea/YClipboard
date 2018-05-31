#pragma once
#include "stdafx.h"

const int MAX_BUFFER_LEN = 1025;

class Client {
private:
	int totalByte;		// Tổng số byte phải gửi
	int sentByte;		// Tổng số byte đã gửi
	char* streamBuffer;	// Dữ liệu
	DWORD streamLength;	// Độ dài của dữ liệu
	SOCKET socket;		// Socket của máy khách
	Client *next;		// Client tiếp theo trong danh sách liên kết

public:
	Client();
	~Client();

	void setSocket(SOCKET socket);	// Lấy socket
	SOCKET getSocket();				// Thiết lập socket

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

	int getClientMessage();			// Lấy thông điệp của máy khách

	Client* getNextClient();			// Lấy Client tiếp theo Client này
	void setNextClient(Client* client);	// Thiết lập Client tiếp theo của Client này
};