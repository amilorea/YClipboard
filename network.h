#pragma once
#include "stdafx.h"
#include "control.h"
#include "client.h"

const int NETWORK_ERROR_SOCKET_FAILURE = -0x1;
const int NETWORK_ERROR_LISTEN_FAILURE = -0x4;
const int NETWORK_ERROR_SOCKET_OPTION_FAILED = -0x2;
const int NETWORK_ERROR_CONNECT_FAILURE = -0x8;
const int NETWORK_ERROR_BINDING_FAILURE = -0x16;
const int NETWORK_ERROR_WSA_STARTUP_FAILURE = -0x32;
const int NETWORK_ERROR_WSA_NOT_AVAILABLE = -0x64;

const int ESTABLISH_ERROR_IO_CONTROL_FAILED = -0x128;
const int ESTABLISH_ERROR_INVALID_IP = -0x256;
const int ESTABLISH_ERROR_CONNECTION_FAILURE = -0x512;
const int ESTABLISH_ERROR_FAIL_TO_SELECT = -0x1024;

const int WRAPPER_ERROR_ACCEPTANCE_FAILURE = -0x1;
const int WRAPPER_ERROR_SELECTION_FAILURE = -0x2;

const int NETWORK_TIMEOUT = 5;

const int NETWORK_MAXIMUM_CLIENT = 30;

class Network {
private:
	static int port;
	Client *root = NULL;

	void initSet(SOCKET);
public:
	static HWND ipBoxhWnd;
	static HWND connectButtonhWnd;
	static HWND disconnectButtonhWnd;
	static int masterSocket;
	fd_set readSet, writeSet, exceptSet;

	Network();
	void init();
	bool establish();

	Client* getClientRoot();
	void addClient(Client*);
	Client* deleteClient(Client *pClient);

	void connectionWrapper(int);
	int getSocketSpecificError(SOCKET);
};

extern Network _network;