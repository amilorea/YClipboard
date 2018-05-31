#include "stdafx.h"
#include "network.h"
Network _network;

int Network::port = 2805;
SOCKET Network::masterSocket = 0;

Network::Network() {
	connectStatus = false;
	ipBox = NULL;
	connectButton = NULL;
	disconnectButton = NULL;
}

void Network::setConnectButton(HWND button) { connectButton = button; }
HWND Network::getConnectButton() { return connectButton; }
void Network::removeConnectButton() {
	if (connectButton != NULL) {
		DestroyWindow(connectButton);
		connectButton = NULL;
	}
}

void Network::setDisconnectButton(HWND button)	{ disconnectButton = button;	}
HWND Network::getDisconnectButton()				{ return disconnectButton;		}
void Network::removeDisconnectButton()	{
	if (disconnectButton != NULL) {
		DestroyWindow(disconnectButton);
		disconnectButton = NULL;
	}
}

void Network::setIpBox(HWND button)		{ ipBox = button;	}
HWND Network::getIpBox()				{ return ipBox;		}
void Network::removeIpBox()	{
	if (ipBox != NULL) {
		DestroyWindow(ipBox);
		ipBox = NULL;
	}
}

void Network::init() {
	WSADATA wsaData;
	try {
		// Thiết lập cấu hình WSA
		int wsaCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (wsaCode == INVALID_SOCKET)
			throw NETWORK_ERROR_WSA_STARTUP_FAILURE;

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
			throw NETWORK_ERROR_WSA_NOT_AVAILABLE;

		// Tạo ra socket cho máy chủ
		Network::masterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Network::masterSocket <= 0)
			throw NETWORK_ERROR_SOCKET_FAILURE;

		struct sockaddr_in address;
		int addrlen = sizeof(address);
		ZeroMemory((char*)&address, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(Network::port);

		// Khớp vào cổng
		if (bind(masterSocket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
			throw NETWORK_ERROR_BINDING_FAILURE;

		if(listen(masterSocket, SOMAXCONN) == SOCKET_ERROR)
			throw NETWORK_ERROR_LISTEN_FAILURE;

		consoleOutput(_T("Now listen on port"), CONSOLE_SPACE);
		consoleOutput(Network::port, CONSOLE_NEWLINE);

		// Khỏi động thread tiếp nhận máy khách
		std::thread wrapper(&Network::connectionWrapper, this, Network::masterSocket);
		wrapper.detach();
	}
	catch (int error) {
		consoleOutput(WSAGetLastError(), CONSOLE_SPACE);
		switch(error) {
		case NETWORK_ERROR_WSA_STARTUP_FAILURE:
			consoleOutput(_T("Network init failed - WSA startup failure"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_WSA_NOT_AVAILABLE:
			consoleOutput(_T("Network init failed - Required WSA not found"), CONSOLE_NEWLINE);
			WSACleanup();
			break;
		case NETWORK_ERROR_SOCKET_FAILURE:
			consoleOutput(_T("Network init failed - Socket init failure"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_BINDING_FAILURE:
			consoleOutput(_T("Network init failed - Binding failure"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_LISTEN_FAILURE:
			consoleOutput(_T("Network init failed - Listen failure"), CONSOLE_NEWLINE);
			closesocket(masterSocket);
			break;
		}
	}
}

void Network::initSet(SOCKET mSocket) {
	// Đặt lại các luồng
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&exceptSet);

	// Gán luồng đọc và luồng lỗi cho mSocket
	FD_SET(mSocket, &readSet);
	FD_SET(mSocket, &exceptSet);

	// Lặp qua các kết nối từ máy khách
	Client* pClient = getClientRoot();
	while (pClient != NULL) {
		if (pClient->getSentByte() < pClient->getTotalByte()) {
			// Ghi
			FD_SET(pClient->getSocket(), &writeSet);
		}
		else {
			// Đọc
			FD_SET(pClient->getSocket(), &readSet);
		}

		// Luồng lỗi
		FD_SET(pClient->getSocket(), &exceptSet);

		// Qua kết nối tiếpt heo
		pClient = pClient->getNextClient();
	}


	consoleOutput(_T("All stream is set"), CONSOLE_NEWLINE);
}
int Network::getSocketSpecificError(SOCKET socket) {
	int optionValue;
	int optionValueLength = sizeof(optionValue);

	getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&optionValue, &optionValueLength);

	return optionValue;
}
void Network::connectionWrapper(SOCKET mSocket) {
	consoleOutput(_T("Wrapper starting now..."), CONSOLE_NEWLINE);

	try {
		while (true) {
			initSet(mSocket);

			if (select(0, &readSet, &writeSet, &exceptSet, 0) > 0) {
				// Phát hiện sự thay đổi trong số các client

				// Nếu có sự thay đổi từ mSocket nghĩa là có kết nối mới
				if (FD_ISSET(mSocket, &readSet)) {
					sockaddr_in clientAddress;
					int clientLength = sizeof(clientAddress);

					// Chấp nhận kết nối mới
					SOCKET newSocket = accept(mSocket, (sockaddr*)&clientAddress, &clientLength);

					if (newSocket == INVALID_SOCKET)
						throw WRAPPER_ERROR_ACCEPTANCE_FAILURE;

					consoleOutput(_T("New client connected at ip"), CONSOLE_SPACE);
					wchar_t buffer[50];
					consoleOutput(InetNtop(AF_INET, &(clientAddress.sin_addr), (PWSTR)buffer, 50), CONSOLE_NEWLINE);

					// Tạo ra một socket không chặn hệ thống
					u_long nNoBlock = 1;
					ioctlsocket(newSocket, FIONBIO, &nNoBlock);

					Client* pClient = new Client;
					pClient->setSocket(newSocket);

					// Thêm socket vừa mới tạo ra vào danh sách liên kết
					addClient(pClient);
				}

				if (FD_ISSET(mSocket, &exceptSet)) {
					consoleOutput(getSocketSpecificError(mSocket), CONSOLE_SPACE);
					consoleOutput(_T("Wrapper exception - Warning: Exception at mSocket"), CONSOLE_NEWLINE);
					continue;
				}

				// Lặp qua danh sách kết nối khách để kiểm tra sự thay đổi của chúng
				Client* pClient = getClientRoot();

				while (pClient != NULL) {
					// Luồng đọc
					if (FD_ISSET(pClient->getSocket(), &readSet)) {
						consoleOutput(_T("Waiting for read"), CONSOLE_NEWLINE);
						pClient->setBufferSize(MAX_BUFFER_LEN);
						// Đọc dữ liệu
						int receivedLength = recv(pClient->getSocket(), pClient->getBuffer(), MAX_BUFFER_LEN, 0);

						if ((0 == receivedLength) || (SOCKET_ERROR == receivedLength)) {
							if (0 != receivedLength) {
								consoleOutput(getSocketSpecificError(pClient->getSocket()), CONSOLE_SPACE);
								consoleOutput(_T("Wrapper exception - Warning: Exception when reading"), CONSOLE_NEWLINE);
							}

							// Loại kết nối khỏi danh sách nếu có lỗi
							pClient = deleteClient(pClient);
							continue;
						}

						// Thiết lập lại bộ đệm
						pClient->setTotalByte(receivedLength);
						pClient->setSentByte(0);

						DWORD message = pClient->getClientMessage();
						// Xử lý yêu cầu
						consoleOutput(_T("Wrapper log - Received message:"), CONSOLE_SPACE);
						consoleOutput(message, CONSOLE_NEWLINE);

						if (message < CLIPSLOT_LOGICAL_PADDING) {
							getSharingClippieceDescription(pClient);
						}
						else {
							DWORD position = message - 0xFF;
							consoleOutput(_T("Download request received at position:"), CONSOLE_SPACE);
							consoleOutput(position, CONSOLE_NEWLINE);
							getSharingClippieceData(pClient, position);
						}
					}
					consoleOutput(_T("Out of read"), CONSOLE_NEWLINE);

					// Luồng ghi
					if (FD_ISSET(pClient->getSocket(), &writeSet)) {
						consoleOutput(_T("Waiting for write"), CONSOLE_NEWLINE);
						int sentLength = 0;

						if (0 < (pClient->getTotalByte() - pClient->getSentByte())) {
							// Ghi dữ liệu
							sentLength = send(pClient->getSocket(),
								(pClient->getBuffer() + pClient->getSentByte()),
								(pClient->getTotalByte() - pClient->getSentByte()),
								0);

							if (sentLength == SOCKET_ERROR) {
								consoleOutput(getSocketSpecificError(pClient->getSocket()), CONSOLE_SPACE);
								consoleOutput(_T("Wrapper exception - Warning: Exception when writing"), CONSOLE_NEWLINE);

								// Loại kết nối khỏi danh sách nếu có lỗi
								pClient = deleteClient(pClient);
								continue;
							}

							if (sentLength == (pClient->getTotalByte() - pClient->getSentByte())) {
								// Đặt lại buffer sau khi ghi xong
								pClient->setTotalByte(0);
								pClient->setSentByte(0);
								pClient->destroyBuffer();
								consoleOutput(_T("Data sent"), CONSOLE_NEWLINE);
							}
							else {
								// Tiếp tục tăng số lượng byte đã gửi với thông điệp chia làm nhiều gói
								pClient->incrSentByte(sentLength);
								consoleOutput(_T("Data keep going"), CONSOLE_NEWLINE);
							}
						}
					}
					consoleOutput(_T("Out of write"), CONSOLE_NEWLINE);

					// Luồng lỗi
					if (FD_ISSET(pClient->getSocket(), &exceptSet)) {
						consoleOutput(getSocketSpecificError(pClient->getSocket()), CONSOLE_SPACE);
						consoleOutput(_T("Wrapper exception - Warning: Exception at client"), CONSOLE_NEWLINE);

						// Loại kết nối khỏi danh sách nếu có lỗi
						pClient = deleteClient(pClient);
						continue;
					}

					// Tiếp tục đi đến kết nối kế
					pClient = pClient->getNextClient();
				}
			}
			else
				throw WRAPPER_ERROR_SELECTION_FAILURE;
		}
	}
	catch (int error) {
		consoleOutput(WSAGetLastError(), CONSOLE_SPACE);
		switch (error) {
		case WRAPPER_ERROR_ACCEPTANCE_FAILURE:
			consoleOutput(_T("Wrapper failure - Exception when accepting new connection"), CONSOLE_NEWLINE);
			break;
		case WRAPPER_ERROR_SELECTION_FAILURE:
			consoleOutput(_T("Wrapper failure - Exception when selecting"), CONSOLE_NEWLINE);
			break;
		}
		return;
	}
}

Client* Network::getClientRoot() {
	return root;
}
void Network::addClient(Client* pClient) {
	// LƯU Ý: Hàm này thêm client mới vào gốc của dãy (thay thế gốc)
	pClient->setNextClient(root);
	root = pClient;
}
Client* Network::deleteClient(Client* pClient) {

	// Trong trường hợp xóa gốc
	if (pClient == root) {
		Client*tClient = root;
		root = root->getNextClient();
		delete tClient;
		return root;
	}

	// Các trường hợp còn lại
	Client*pPrevious = root;
	Client*pCurrent = root->getNextClient();

	while (pCurrent != NULL) {
		if (pCurrent == pClient) {
			Client*pTemp = pCurrent->getNextClient();
			pPrevious->setNextClient(pTemp);
			delete pCurrent;
			return pTemp;
		}

		pPrevious = pCurrent;
		pCurrent = pCurrent->getNextClient();
	}

	return NULL;
}

bool Network::establish() {
	consoleOutput(_T("Now connecting"), CONSOLE_NEWLINE);
	TIMEVAL Timeout;
	Timeout.tv_sec = NETWORK_TIMEOUT_LIMIT;
	Timeout.tv_usec = 0;

	wchar_t buffer[50];
	GetWindowText(getIpBox(), buffer, 50);

	struct sockaddr_in serverAddress;
	memset(&serverAddress, '0', sizeof(serverAddress));
	int ip;

	try {
		socketToServer = socket(AF_INET, SOCK_STREAM, 0);
		if (socketToServer < 0)
			throw NETWORK_ERROR_SOCKET_FAILURE;

		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(Network::port);

		// Dịch địa chỉ IPv4 (IPv6) từ dạng chuỗi ký tự sang số
		ip = InetPton(AF_INET, buffer, &serverAddress.sin_addr);
		if (ip <= 0)
			throw ESTABLISH_ERROR_INVALID_IP;

		// Thiết lập chế độ non-blocking cho việc kết nối
		unsigned long iMode = 1;
		int iResult = ioctlsocket(socketToServer, FIONBIO, &iMode);
		if (iResult != NO_ERROR)
			throw ESTABLISH_ERROR_IO_CONTROL_FAILED;

		connect(socketToServer, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

		// Thiết lập lại chế độ block cho kết nối?
		iMode = 0;
		iResult = ioctlsocket(socketToServer, FIONBIO, &iMode);
		if (iResult != NO_ERROR)
			throw ESTABLISH_ERROR_IO_CONTROL_FAILED;

		fd_set Write, Err;
		FD_ZERO(&Write);
		FD_ZERO(&Err);
		FD_SET(socketToServer, &Write);
		FD_SET(socketToServer, &Err);

		// Kiểm tra lại kết nối
		select(0, NULL, &Write, &Err, &Timeout);
		if (FD_ISSET(socketToServer, &Write) == 0)
			throw ESTABLISH_ERROR_CONNECTION_FAILURE;

		char sendBuffer[MAX_BUFFER_LEN];
		ZeroMemory(sendBuffer, MAX_BUFFER_LEN);
		insertDWORD(sendBuffer, NETWORK_MESSAGE_HELLO);
		// Gửi yêu cầu lấy về mô tả clippiece của máy chủ
		send(socketToServer, sendBuffer, 4, 0);

		MessageBox(Window::globalhWnd, _T("Connection is established!"), _T("Succeed"), MB_OK);

		return true;
	}
	catch (int error) {
		consoleOutput(WSAGetLastError(), CONSOLE_SPACE);
		switch (error) {
		case NETWORK_ERROR_SOCKET_FAILURE:
			consoleOutput(_T("Establish failed - Socket creation failure"), CONSOLE_NEWLINE);
			break;
		case ESTABLISH_ERROR_INVALID_IP:
			consoleOutput(_T("Establish failed - Invalid IP provided"), CONSOLE_NEWLINE);
			break;
		case ESTABLISH_ERROR_CONNECTION_FAILURE:
			consoleOutput(_T("Establish failed - Connection not established"), CONSOLE_NEWLINE);
			break;
		case ESTABLISH_ERROR_IO_CONTROL_FAILED:
			consoleOutput(_T("Establish failed - Stream set failure"), CONSOLE_NEWLINE);
			break;
		}
		MessageBox(Window::globalhWnd, _T("Connection is not established!"), _T("Failure"), MB_OK);
		return switchConnect();
	}
	return false;
}
bool Network::disconnect() {
	consoleOutput(_T("Now disconnecting"), CONSOLE_NEWLINE);
	if (closesocket(socketToServer))
		consoleOutput(_T("Connection to server closed"), CONSOLE_NEWLINE);

	MessageBox(Window::globalhWnd, _T("Connection is removed!"), _T("Disconnected"), MB_OK);
	return false;
}
bool Network::switchConnect() {
	if (connectStatus) {
		connectStatus = false;
		ShowWindow(connectButton, SW_SHOW);
		EnableWindow(connectButton, TRUE);
		ShowWindow(disconnectButton, SW_HIDE);
		EnableWindow(disconnectButton, FALSE);
		return disconnect();
	}
	else {
		connectStatus = true;
		ShowWindow(disconnectButton, SW_SHOW);
		EnableWindow(disconnectButton, TRUE);
		ShowWindow(connectButton, SW_HIDE);
		EnableWindow(connectButton, FALSE);
		return establish();
	}
}

bool Network::getClippieceDescription() {
	try {
		DWORD totalSize;
		char* storage = getData(&totalSize);

		// Phân giải
		// Bỏ qua DWORD đầu tiên (chứa kích thước của gói dữ liệu đến)
		DWORD iterator = sizeof(DWORD);
		DWORD subStructSize;
		DWORD dataSize;
		int logicPosition;
		while (iterator < totalSize) {
			// Tái tạo lại phần 1 - Độ dài toàn cấu trúc
			subStructSize = makeDWORD(storage[0 + iterator], storage[1 + iterator], storage[2 + iterator], storage[3 + iterator]);
			iterator += sizeof(DWORD);

			// Tái tạo lại phần 2 - Vị trí logic của clippiece ở trên máy đang chia sẻ nó
			logicPosition = makeDWORD(storage[0 + iterator], storage[1 + iterator], storage[2 + iterator], storage[3 + iterator]);
			iterator += sizeof(DWORD);

			// Tái tạo lại phần 3 - Dữ liệu mô tả
			dataSize = subStructSize - sizeof(DWORD) - sizeof(DWORD);
			_clipslot.createClippieceFromSharedDescription(dataSize, logicPosition, storage + iterator);
			iterator += dataSize;
		}
		free(storage);
		return true;
	}
	catch (int error) {
		switch (error) {
		case NETWORK_ERROR_TRANSFER_FAILED:
			consoleOutput(_T("Receive clippiece description failed"), CONSOLE_NEWLINE);
		}
		return false;
	}
}
char* Network::getClippieceData(DWORD* totalSize)
{
	try {
		char* storage = getData(totalSize);
		return storage;
	}
	catch (int error) {
		switch (error) {
		case NETWORK_ERROR_TRANSFER_FAILED:
			consoleOutput(_T("Receive clippiece data failed"), CONSOLE_NEWLINE);
		}
		*totalSize = 0;
		return NULL;
	}
}

void Network::getSharingClippieceDescription(Client* client) {
	DWORD storageSize;
	char* storage = _clipslot.collectClippieceDescription(&storageSize);
	// Đẩy dữ liệu vào bộ đệm chờ gửi
	client->setBufferSize(storageSize);
	client->setBufferContent(storage, storageSize);
	client->setTotalByte(storageSize);
	free(storage);
	storage = NULL;
}
void Network::getSharingClippieceData(Client* client, DWORD position) {
	DWORD storageSize;
	char* storage = _clipslot.collectClippieceData(&storageSize, position);
	// Đẩy dữ liệu vào bộ đệm chờ gửi
	client->setBufferSize(storageSize);
	client->setBufferContent(storage, storageSize);
	client->setTotalByte(storageSize);
	free(storage);
	storage = NULL;
}

char* Network::getData(DWORD* totalSize) {
	TIMEVAL Timeout;
	Timeout.tv_sec = NETWORK_TIMEOUT_LIMIT;
	Timeout.tv_usec = 0;

	fd_set Read;
	char buffer[MAX_BUFFER_LEN];
	char* receivedData = NULL;
	int byteReceived;
	DWORD currentDownloadedSize = 0;
	DWORD byteExpected = 0;
	try {
		// Vòng lặp vô hạn lấy dữ liệu liên tục
		while (true) {
			// Khởi tạo các luồng đọc
			FD_ZERO(&Read);
			FD_SET(socketToServer, &Read);
			select(socketToServer, &Read, NULL, NULL, &Timeout);
			if (FD_ISSET(socketToServer, &Read) == 0)
				throw NETWORK_ERROR_CONNECTION_ERROR;

			// Bắt đầu đọc
			byteReceived = recv(socketToServer, buffer, MAX_BUFFER_LEN, 0);
			if (byteExpected == 0) {
				// Trong lần đọc đầu tiên ta sẽ lấy độ dài của toàn bộ khối dữ liệu mà ta sẽ đọc
				byteExpected = makeDWORD(buffer[0], buffer[1], buffer[2], buffer[3]);
				receivedData = (char*)malloc(byteExpected);
				if (receivedData == NULL)
					throw ALLOCATION_FAILED;
				*totalSize = byteExpected;
			}

			if (byteReceived == 0)
				throw NETWORK_ERROR_CONNECTION_CLOSED;

			if (byteReceived < 0)
				throw NETWORK_ERROR_CONNECTION_ERROR;

			memcpy(receivedData + currentDownloadedSize, buffer, byteReceived);
			currentDownloadedSize += byteReceived;
			if (byteExpected == currentDownloadedSize)
				break;
		}

		return receivedData;
	}
	catch (int error) {
		consoleOutput(WSAGetLastError(), CONSOLE_SPACE);
		switch (error) {
		case NETWORK_ERROR_CONNECTION_ERROR:
			consoleOutput(_T("Receive data failed - Connection error"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_CONNECTION_CLOSED:
			consoleOutput(_T("Receive data failed - Transfer end unexpectedly"), CONSOLE_NEWLINE);
			break;
		case ALLOCATION_FAILED:
			consoleOutput(_T("Receive data failed - Allocation failure"), CONSOLE_NEWLINE);
			break;
		}
		*totalSize = 0;
		free(receivedData);
		throw NETWORK_ERROR_TRANSFER_FAILED;
		return NULL;
	}
}
bool Network::downloadToPosition(DWORD position, DWORD logicalPosition) {
	TIMEVAL Timeout;
	Timeout.tv_sec = NETWORK_TIMEOUT_LIMIT;
	Timeout.tv_usec = 0;

	try {
		// Khởi tạo các luồng ghi
		fd_set Write, Err;
		FD_ZERO(&Write);
		FD_ZERO(&Err);
		FD_SET(socketToServer, &Write);
		FD_SET(socketToServer, &Err);

		// Kiểm tra lại kết nối
		select(0, NULL, &Write, &Err, &Timeout);
		if (FD_ISSET(socketToServer, &Write) == 0)
			throw ESTABLISH_ERROR_CONNECTION_FAILURE;

		char sendBuffer[MAX_BUFFER_LEN];
		ZeroMemory(sendBuffer, MAX_BUFFER_LEN);
		insertDWORD(sendBuffer, CLIPSLOT_LOGICAL_PADDING + logicalPosition);
		// Gửi vị trí muốn tải về tới máy chủ
		send(socketToServer, sendBuffer, 4, 0);
		DWORD totalSize;
		// Lấy dữ liệu về
		char* storage = getClippieceData(&totalSize);

		if (totalSize == 0)
			throw NETWORK_ERROR_TRANSFER_FAILED;

		// Tạo ra clippiece từ dữ liệu nhận được
		_clipslot.addSharedDataToClippiece(totalSize, storage, position);
		MessageBox(Window::globalhWnd, _T("Download completed!"), _T("Succeed"), MB_OK);
		return true;
	}
	catch (int error) {
		consoleOutput(WSAGetLastError(), CONSOLE_SPACE);
		switch (error) {
		case ESTABLISH_ERROR_CONNECTION_FAILURE:
			consoleOutput(_T("Download failed - Connection not established"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_TRANSFER_FAILED:
			consoleOutput(_T("Download failed - Transfer failed"), CONSOLE_NEWLINE);
			break;
		}
		MessageBox(Window::globalhWnd, _T("Download failed!"), _T("Failure"), MB_OK);
		return false;
	}
}

