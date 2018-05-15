#pragma once
#include "stdafx.h"
#include "client.h"

Client::Client() {
	socket = SOCKET_ERROR;
	ZeroMemory(streamBuffer, MAX_BUFFER_LEN);
	totalByte = 0;
	sentByte = 0;
	next = NULL;
}

Client::~Client() {
	closesocket(socket);
}

void Client::setTotalByte(int n) {
	totalByte = n;
}

int Client::getTotalByte() {
	return totalByte;
}

void Client::setSentByte(int n) {
	sentByte = n;
}

void Client::incrSentByte(int n) {
	sentByte += n;
}

int Client::getSentByte() {
	return sentByte;
}

void Client::setSocket(SOCKET s) {
	socket = s;
}

SOCKET Client::getSocket() {
	return socket;
}

void Client::setBuffer(char *sBuffer) {
	strcpy_s(streamBuffer, sBuffer);
}

//void Client::getBuffer(char *sBuffer) {
//	strcpy_s(sBuffer, streamBuffer);
//}

char* Client::getBuffer() {
	return streamBuffer;
}

void Client::zeroBuffer() {
	ZeroMemory(streamBuffer, MAX_BUFFER_LEN);
}

Client* Client::getNextClient()
{
	return next;
}

void Client::setNextClient(Client *pNext)
{
	next = pNext;
}