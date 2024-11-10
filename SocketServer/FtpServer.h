#pragma once

#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <map>
#include <mutex>

#include "FtpDatabase.h"
#include "ClientSession.h"

#define BUFFER_SIZE 4096

class FtpServer
{
	friend class ClientSession;
public:
	FtpServer(const char* address, std::string fileStorage, UINT port = IPPORT_FTP, UINT bufferSize = 4096);
	~FtpServer();
	bool start();
	void stop();

	std::string filesStorage;
	UINT bufferSize;
	void setDatabase(FtpDatabase* database);
private:
	const char* address;
	UINT port;
	FtpDatabase* database;
	std::map<SOCKET, ClientSession> sessions;

	std::atomic<bool> serverRunning;
};