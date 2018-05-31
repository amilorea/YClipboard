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

const int NETWORK_ERROR_CONNECTION_CLOSED		= 0xF0;	// Kết nối bị ngắt
const int NETWORK_ERROR_TRANSFER_FAILED			= 0xF1;	// Truyền dữ liệu thất bại

const int NETWORK_TIMEOUT_LIMIT = 5;		// Thời gian chờ tối đa của máy khách trước khi Timeout
const int NETWORK_MAXIMUM_CLIENT = 30;		// Số máy khách cho phép kết nối tối đa

class Network {
private:
	fd_set readSet, writeSet, exceptSet;	// Các luồng

	SOCKET socketToServer;					// Socket kết nối tới máy chủ
	Client *root = NULL;

	HWND ipBox;				// Hộp nhập IP và các nút kết nối / ngắt kết nối
	HWND connectButton;
	HWND disconnectButton;

	bool connectStatus;		// Trạng thái kết nối

	void initSet(SOCKET);					// Khởi tạo các luồng giao tiếp
	void connectionWrapper(SOCKET mSocket);	// Hàm xử lý giao tiếp giữa máy trạm và máy chủ
	int getSocketSpecificError(SOCKET);
public:
	static int port;
	static SOCKET masterSocket;				// Socket mà máy khách kết nối tới

	Network();
	void init();							// Thiết lập chương trình như là một máy chủ
	char* getData(DWORD* totalSize);								// Đọc dữ liệu trả về từ máy chủ

	// Các hàm thêm / lấy / xóa nút bấm và hộp nhập
	void setConnectButton(HWND button);
	HWND getConnectButton();
	void removeConnectButton();

	void setDisconnectButton(HWND button);
	HWND getDisconnectButton();
	void removeDisconnectButton();

	void setIpBox(HWND box);
	HWND getIpBox();
	void removeIpBox();

	// Các hàm lấy gốc của danh sách liên kết, thêm / xóa Client trong danh sách
	Client* getClientRoot();
	void addClient(Client* pClient);
	Client* deleteClient(Client* pClient);

	bool establish();							// Kết nối
	bool disconnect();							// Ngắt kết nối
	bool switchConnect();						// Chuyển tình trạng kết nối

	bool getClippieceDescription();				// Lấy mô tả từ clippiece được chia sẻ
	char* getClippieceData(DWORD* totalSize);	// Lấy dữ liệu từ clippiece được chia sẻ

	void getSharingClippieceDescription(Client* client);			// Lấy mô tả từ clippiece đang chia sẻ
	void getSharingClippieceData(Client* client, DWORD position);	// Lấy dữ liệu từ clippiece đang chia sẻ

	bool downloadToPosition(DWORD position, DWORD logicalPosition);	// Tải Clippiece ở vị trí cho trước
};

extern Network _network;