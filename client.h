#pragma once
#include "stdafx.h"

const int MAX_BUFFER_LEN = 1025;

class Client {
private:
	int totalByte;
	int sentByte;
	SOCKET socket;  // Socket
	char streamBuffer[MAX_BUFFER_LEN];
	Client *next;

public:
	Client();
	~Client();

	void setTotalByte(int);
	int getTotalByte();

	void setSentByte(int);
	void incrSentByte(int);
	int getSentByte();

	void setSocket(SOCKET);
	SOCKET getSocket();

	void setBuffer(char*);
	//void getBuffer(char*);
	char* getBuffer();
	void zeroBuffer();

	Client* getNextClient();
	void setNextClient(Client*);
};