#include "stdafx.h"
#include "network.h"
Network _network;

HWND Network::ipBoxhWnd = 0;
HWND Network::connectButtonhWnd = 0;
HWND Network::disconnectButtonhWnd = 0;

int Network::port = 2805;
int Network::masterSocket = 0;

Network::Network() {

}

void Network::init() {
	char opt = 1;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	WSADATA wsaData;
	try {
		int wsaCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (wsaCode == INVALID_SOCKET)
			throw NETWORK_ERROR_WSA_STARTUP_FAILURE;

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
			throw NETWORK_ERROR_WSA_NOT_AVAILABLE;

		Network::masterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Network::masterSocket <= 0)
			throw NETWORK_ERROR_SOCKET_FAILURE;

		//int socketoptStatusCode = setsockopt(Network::masterSocket,
		//	IPPROTO_TCP,
		//	TCP_,
		//	&opt,
		//	sizeof(opt));
		//if (socketoptStatusCode != 0)
		//	throw NETWORK_ERROR_SOCKET_OPTION_FAILED;

		ZeroMemory((char *)&address, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(Network::port);

		// Khớp vào cổng
		if (bind(masterSocket, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
			throw NETWORK_ERROR_BINDING_FAILURE;

		if(listen(masterSocket, SOMAXCONN) == SOCKET_ERROR)
			throw NETWORK_ERROR_LISTEN_FAILURE;
		consoleOutput(_T("Now listen on port"), CONSOLE_SPACE);
		consoleOutput(Network::port, CONSOLE_NEWLINE);

		std::thread wrapper(&Network::connectionWrapper, this, Network::masterSocket);
		wrapper.detach();
	}
	catch (int error) {
		consoleOutput(WSAGetLastError(), CONSOLE_SPACE);
		switch(error) {
		case NETWORK_ERROR_SOCKET_FAILURE:
			consoleOutput(_T("Network init failed - Error: Socket init failed, error code"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_WSA_STARTUP_FAILURE:
			consoleOutput(_T("Network init failed - Error: WSA startup failure, error code"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_SOCKET_OPTION_FAILED:
			consoleOutput(_T("Network init failed - Error: Socket option set failed, error code"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_BINDING_FAILURE:
			consoleOutput(_T("Network init failed - Error: Binding failed"), CONSOLE_NEWLINE);
			break;
		case NETWORK_ERROR_WSA_NOT_AVAILABLE:
			consoleOutput(_T("Network init failed - Error: Required WSA not found"), CONSOLE_NEWLINE);
			WSACleanup();
			break;
		case NETWORK_ERROR_LISTEN_FAILURE:
			consoleOutput(_T("Network init failed - Error: Listen failure"), CONSOLE_NEWLINE);
			closesocket(masterSocket);
			break;
		}
	}
}

void Network::initSet(SOCKET masterSocket) {

	// Khởi tạo các luồng
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&exceptSet);

	// Gán luồng đọc và luồng lỗi cho masterSocket
	FD_SET(masterSocket, &readSet);
	FD_SET(masterSocket, &exceptSet);

	consoleOutput(_T("All stream is set"), CONSOLE_NEWLINE);

	// Gán các luồng vào socket trong dãy
	Client *pClient = getClientRoot();

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

		//Move to next node on the list
		pClient = pClient->getNextClient();
	}

	consoleOutput(_T("Client list formed"), CONSOLE_NEWLINE);
}

int Network::getSocketSpecificError(SOCKET socket) {
	int optionValue;
	int optionValueLength = sizeof(optionValue);

	getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&optionValue, &optionValueLength);

	return optionValue;
}

void Network::connectionWrapper(int masterSocket) {
	consoleOutput(_T("Wrapper starting now..."), CONSOLE_NEWLINE);

	try {
		while (true) {
			initSet(masterSocket);

			if (select(0, &readSet, &writeSet, &exceptSet, 0) > 0) {
				// Phát hiện sự thay đổi trong số các client

				// Nếu có sự thay đổi từ masterSocket nghĩa là có kết nối mới
				if (FD_ISSET(masterSocket, &readSet)) {
					sockaddr_in clientAddress;
					int clientLength = sizeof(clientAddress);

					// Chấp nhậnk kết nối mới
					SOCKET newSocket = accept(masterSocket, (sockaddr*)&clientAddress, &clientLength);

					if (newSocket == INVALID_SOCKET)
						throw WRAPPER_ERROR_ACCEPTANCE_FAILURE;

					consoleOutput("New client connected at ip", CONSOLE_SPACE);
					wchar_t buffer[50];
					consoleOutput(InetNtop(AF_INET, &(clientAddress.sin_addr), (PWSTR)buffer, 50), CONSOLE_NEWLINE);

					// Tạo ra một socket không chặn hệ thống
					u_long nNoBlock = 1;
					ioctlsocket(newSocket, FIONBIO, &nNoBlock);

					Client *pClient = new Client;
					pClient->setSocket(newSocket);

					// Thêm socket vừa mới tạo ra vào danh sách liên kết
					addClient(pClient);
				}

				if (FD_ISSET(masterSocket, &exceptSet)) {
					consoleOutput(getSocketSpecificError(masterSocket), CONSOLE_SPACE);
					consoleOutput("Wrapper exception - Warning: Exception at masterSocket", CONSOLE_NEWLINE);
					continue;
				}

				// Lặp qua danh sách client để kiểm tra sự thay đổi của chúng
				Client *pClient = getClientRoot();

				while (pClient != NULL) {
					// Luồng đọc
					if (FD_ISSET(pClient->getSocket(), &readSet)) {
						int receivedLength = recv(pClient->getSocket(), pClient->getBuffer(), MAX_BUFFER_LEN, 0);

						if ((0 == receivedLength) || (SOCKET_ERROR == receivedLength)) {
							if (0 != receivedLength) {
								consoleOutput(getSocketSpecificError(pClient->getSocket()), CONSOLE_SPACE);
								consoleOutput("Wrapper exception - Warning: Exception when reading", CONSOLE_NEWLINE);
								continue;
							}

							// Loại client khỏi danh sách nếu có lỗi
							pClient = deleteClient(pClient);
							continue;
						}

						// Thiết lập bộ đệm
						pClient->setTotalByte(receivedLength);
						pClient->setSentByte(0);

						consoleOutput("Wrapper log - Received message:", CONSOLE_SPACE);
						consoleOutput(pClient->getBuffer(), CONSOLE_NEWLINE);
					}

					// Luồng ghi
					if (FD_ISSET(pClient->getSocket(), &writeSet)) {
						int sentLength = 0;

						if (0 < (pClient->getTotalByte() - pClient->getSentByte())) {
							sentLength = send(pClient->getSocket(),
								(pClient->getBuffer() + pClient->getSentByte()),
								(pClient->getTotalByte() - pClient->getSentByte()),
								0);

							if (sentLength == SOCKET_ERROR) {
								consoleOutput(getSocketSpecificError(pClient->getSocket()), CONSOLE_SPACE);
								consoleOutput("Wrapper exception - Warning: Exception when writing", CONSOLE_NEWLINE);

								// Loại client khỏi danh sách nếu có lỗi
								pClient = deleteClient(pClient);
								continue;
							}

							if (sentLength == (pClient->getTotalByte() - pClient->getSentByte())) {
								// Đặt lại buffer sau khi ghi xong
								pClient->setTotalByte(0);
								pClient->setSentByte(0);
							}
							else {
								// Tiếp tục tăng số lượng byte đã gửi với thông điệp chia làm nhiều gói
								pClient->incrSentByte(sentLength);
							}
						}
					}

					// Luồng lỗi
					if (FD_ISSET(pClient->getSocket(), &exceptSet)) {
						consoleOutput(getSocketSpecificError(pClient->getSocket()), CONSOLE_SPACE);
						consoleOutput("Wrapper exception - Warning: Exception at client", CONSOLE_NEWLINE);

						// Loại client khỏi danh sách nếu có lỗi
						pClient = deleteClient(pClient);
						continue;
					}

					// Tiếp tục đi đến client kế
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
			consoleOutput(_T("Wrapper failure - Error: Exception when accept new connection"), CONSOLE_NEWLINE);
			break;
		case WRAPPER_ERROR_SELECTION_FAILURE:
			consoleOutput(_T("Wrapper failure - Error: Exception when selecting"), CONSOLE_NEWLINE);
			break;
		}
		return;
	}
}

bool Network::establish() {
	TIMEVAL Timeout;
	Timeout.tv_sec = NETWORK_TIMEOUT;
	Timeout.tv_usec = 0;

	wchar_t buffer[50];
	GetWindowText(Network::ipBoxhWnd, buffer, 50);

	struct sockaddr_in address;

	struct sockaddr_in serverAddress;
	memset(&serverAddress, '0', sizeof(serverAddress));

	int soc;
	int ip;

	try {
		soc = socket(AF_INET, SOCK_STREAM, 0);
		if (soc < 0)
			throw NETWORK_ERROR_SOCKET_FAILURE;

		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(Network::port);
		ip = InetPton(AF_INET, buffer, &serverAddress.sin_addr);
		if (ip <= 0)
			throw ESTABLISH_ERROR_INVALID_IP;

		// Thiết lập chế độ non-blocking cho việc kết nối
		unsigned long iMode = 1;
		int iResult = ioctlsocket(soc, FIONBIO, &iMode);
		if (iResult != NO_ERROR)
			throw ESTABLISH_ERROR_IO_CONTROL_FAILED;

		// Dịch địa chỉ IPv4 (IPv6) từ dạng chuỗi ký tự sang số

		connect(soc, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

		//send(sock, hello, strlen(hello), 0);
		//printf("Hello message sent\n");
		//valread = read(sock, buffer, 1024);
		//printf("%s\n", buffer);

		// Thiết lập lại chế độ block cho kết nối?
		iMode = 0;
		iResult = ioctlsocket(soc, FIONBIO, &iMode);
		if (iResult != NO_ERROR)
			throw ESTABLISH_ERROR_IO_CONTROL_FAILED;

		fd_set Write, Err;
		FD_ZERO(&Write);
		FD_ZERO(&Err);
		FD_SET(soc, &Write);
		FD_SET(soc, &Err);

		// Kiểm tra lại kết nối
		select(0, NULL, &Write, &Err, &Timeout);
		if (FD_ISSET(soc, &Write) == 0)
			throw ESTABLISH_ERROR_CONNECTION_FAILURE;

		consoleOutput(_T("Connection to"), CONSOLE_SPACE);
		consoleOutput(buffer, CONSOLE_SPACE);
		consoleOutput(_T("is established."), CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		consoleOutput(WSAGetLastError(), CONSOLE_SPACE);
		switch (error) {
		case NETWORK_ERROR_SOCKET_FAILURE:
			consoleOutput(_T("Establish failed - Error: Socket creation failed"), CONSOLE_NEWLINE);
			break;
		case ESTABLISH_ERROR_INVALID_IP:
			consoleOutput(_T("Establish failed - Error: Invalid IP provided"), CONSOLE_NEWLINE);
			break;
		case ESTABLISH_ERROR_CONNECTION_FAILURE:
			consoleOutput(_T("Establish failed - Error: Connection not established"), CONSOLE_NEWLINE);
			break;
		case ESTABLISH_ERROR_IO_CONTROL_FAILED:
			consoleOutput(_T("Establish failed - Error: Cannot set time out control"), CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
	return false;
}

Client* Network::getClientRoot() {
	return root;
}

void Network::addClient(Client * pClient) {
	// LƯU Ý: Hàm này thêm client mới vào gốc của dãy (thay thế gốc)
	pClient->setNextClient(root);
	root = pClient;
}

Client* Network::deleteClient(Client * pClient) {

	// Trong trường hợp xóa gốc
	if (pClient == root)
	{
		Client *tClient = root;
		root = root->getNextClient();
		delete tClient;
		return root;
	}

	// Các trường hợp còn lại
	Client *pPrevious = root;
	Client *pCurrent = root->getNextClient();

	while (pCurrent != NULL) {
		if (pCurrent == pClient) {
			Client *pTemp = pCurrent->getNextClient();
			pPrevious->setNextClient(pTemp);
			delete pCurrent;
			return pTemp;
		}

		pPrevious = pCurrent;
		pCurrent = pCurrent->getNextClient();
	}

	return NULL;
}
