#pragma once
#pragma comment(lib, "Ws2_32.lib")
#include "stdafx.h"
#include "WindowsProject1.h"
#include "client.h"
#include "clipslot.h"

const int NETWORK_MESSAGE_HELLO = 0x1;

const int NETWORK_ERROR_WSA_STARTUP_FAILURE		= 0x1;	// Không khởi động được WSA
const int NETWORK_ERROR_WSA_NOT_AVAILABLE		= 0x2;	// Không có phiên bản WSA phù hợp
const int NETWORK_ERROR_SOCKET_FAILURE			= 0x3;	// Không tạo được socket
const int NETWORK_ERROR_BINDING_FAILURE			= 0x4;	// Không thể khớp được socket vào cổng
const int NETWORK_ERROR_LISTEN_FAILURE			= 0x5;	// Không thể lắng nghe từ cổng
const int NETWORK_ERROR_CONNECTION_ERROR		= 0x6;	// Kết nối lỗi

const int WRAPPER_ERROR_ACCEPTANCE_FAILURE		= 0xF1;	// Không thể chấp nhận kết nối mới
const int WRAPPER_ERROR_SELECTION_FAILURE		= 0xF2;	// Không thể lựa chọn luồng phù hợp

const int ESTABLISH_ERROR_CONNECTION_FAILURE	= 0x7;	// Không thể kết nối
const int ESTABLISH_ERROR_IO_CONTROL_FAILED		= 0x8;	// Nhập xuất lỗi
const int ESTABLISH_ERROR_INVALID_IP			= 0x9;	// IP không đúng

const int NETWORK_ERROR_CONNECTION_CLOSED		= 0xF0;
const int NETWORK_ERROR_TRANSFER_FAILED			= 0xF1;

const int NETWORK_TIMEOUT_LIMIT = 5;
const int NETWORK_MAXIMUM_CLIENT = 30;

class Network {
private:
	fd_set readSet, writeSet, exceptSet;

	int socketToServer;
	Client *root = NULL;

	HWND ipBox;
	HWND connectButton;
	HWND disconnectButton;

	bool connectStatus;

	void initSet(SOCKET);					// Khởi tạo các luồng giao tiếp
	void connectionWrapper(SOCKET mSocket);	// Hàm xử lý giao tiếp giữa máy trạm và máy chủ
	int getSocketSpecificError(SOCKET);
public:
	static int port;
	static SOCKET masterSocket;

	Network();
	void init();		// Thiết lập chương trình như là một máy chủ

	void setConnectButton(HWND button);
	HWND getConnectButton();
	void removeConnectButton();

	void setDisconnectButton(HWND button);
	HWND getDisconnectButton();
	void removeDisconnectButton();

	void setIpBox(HWND box);
	HWND getIpBox();
	void removeIpBox();

	Client* getClientRoot();
	void addClient(Client* pClient);
	Client* deleteClient(Client* pClient);

	bool establish();
	bool disconnect();
	bool switchConnect();

	bool getClippieceDescription();				// Lấy mô tả từ clippiece được chia sẻ
	char* getClippieceData(DWORD* totalSize);	// Lấy dữ liệu từ clippiece được chia sẻ

	void getSharingClippieceDescription(Client* client);			// Lấy mô tả từ clippiece đang chia sẻ
	void getSharingClippieceData(Client* client, DWORD position);	// Lấy dữ liệu từ clippiece đang chia sẻ

	bool downloadToPosition(DWORD position, DWORD logicalPosition);
	char* getData(DWORD* totalSize);
};

extern Network _network;