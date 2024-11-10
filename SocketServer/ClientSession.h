#pragma once
#include "WinSock2.h"

#include "FtpServer.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <fstream> 
#include <memory>
#include <string>

#define FILE_BUFFER_SIZE 1024

class FtpServer;

typedef enum TRANSFER_FORMAT
{
	BINARY, ASCII, NONE
} TRANSFER_FORMAT;

typedef enum TRANSFER_TYPE
{
	ACTIVE, PASSIVE, DISABLED
} TRANSFER_TYPE;

class ClientSession {

public:
	ClientSession() {
	};
	ClientSession(SOCKET socket, int clientPort, std::string clientAddress, FtpServer* server) {
		this->commandSocket = socket;
		this->currentDirectory = NULL;
		this->clientPort = clientPort;
		this->clientAddress = clientAddress;
		this->user = NULL;
		this->dataSocket = NULL;
		this->server = server;
		this->userSocket = NULL;
		this->transferType = TRANSFER_TYPE::DISABLED;
		this->transferFormat = TRANSFER_FORMAT::NONE;
		this->isAuthorized = false;
	}

	bool isAuthorized;
	SOCKET commandSocket;
	SOCKET dataSocket;
	SOCKET userSocket;
	std::shared_ptr<Directory> currentDirectory;
	int clientPort;
	std::string clientAddress;
	std::shared_ptr<User> user;
	FtpServer* server;

	TRANSFER_FORMAT transferFormat;
	TRANSFER_TYPE transferType;


	void handleCommandSocket();
	bool createPassiveSocket();
	void handleDataSocket();

	void sendDirectoryList();
	void sendDirectoryNameList();
	std::string createPath();
	void sendFileData(std::string& filePath);

	void getFileData(std::string& downloadPath);
};
