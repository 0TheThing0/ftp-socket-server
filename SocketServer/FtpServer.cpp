#include "FtpServer.h"
#include "ClientSession.h"

FtpServer::FtpServer(const char* address, std::string fileStorage, UINT port, UINT bufferSize) {
	this->address = address;
	this->port = port;
	this->bufferSize = bufferSize;
	this->filesStorage = fileStorage;
}

void FtpServer::stop() {
	serverRunning = false;
}
FtpServer::~FtpServer(){}

bool FtpServer::start() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	this->serverRunning = true;

	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo* ptr = NULL;

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		return false;
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr)); // Обнуление структуры
	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_port = htons(this->port);

	inet_pton(AF_INET, this->address, &server_addr.sin_addr);

	// Setup the TCP listening socket
	iResult = bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN addr;
	int len = sizeof(addr);
	iResult = getsockname(serverSocket, (struct sockaddr*)&addr, &len);
	std::cout << "Server listening on " << this->address << ":" << ntohs(addr.sin_port) << std::endl;


	while (serverRunning) {
		sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);

		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Error accepting connection.\n";
			continue;
		}

		char ipStr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr)); // Преобразование адреса в строку

		{
			sessions[clientSocket] = ClientSession(clientSocket, clientAddr.sin_port, ipStr, this);
		}
		std::thread(&ClientSession::handleCommandSocket, sessions[clientSocket]).detach(); // Обработка клиента в отдельном потоке
	}

	closesocket(serverSocket);
	WSACleanup(); // Завершение работы Winsock
	return 0;
}

void FtpServer::setDatabase(FtpDatabase* database) {
	this->database = database;
}