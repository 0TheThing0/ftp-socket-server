#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <map>
#include <mutex>

#include "FtpServer.h"
#include "FtpDatabase.h"
#include "Permission.h"
#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 4096

FtpServer* server;
FtpDatabase database;

void handleConsoleCommand() {
	std::string command;
	while (true) {
		std::cout << "Enter command (start, stop, restart, status, exit): ";
		std::getline(std::cin, command);
	}
}


int main() {
	server = new FtpServer("127.0.0.1","D:\\ServerDir\\", 454, 4096);
	server->setDatabase(&database);



	Group group1("test");

	User user1("anonymus", "1234",&group1);
	User user2("tester", "4321",&group1);
	
	std::shared_ptr<Directory> dir(std::make_shared<Directory>("fadsf", &user1, &group1, ALL, RW, READ));
	std::shared_ptr<Directory> dir2(std::make_shared<Directory>("dir2", &user1, &group1, ALL, RW, READ,dir));
	std::shared_ptr<Directory> dir3(std::make_shared<Directory>("dir3", &user1, &group1, ALL, RW, READ,dir2));


	dir->addSubdirectory(dir2);
	dir2->addSubdirectory(dir3);

	File file1("test.f", "D:\\Учёба\\МатПрог\\ЛР6МарковскийВ16.xlsx", &user1, &group1, ALL, ALL, ALL);
	dir->addFile(std::make_shared<File>(file1));

	user1.setRootDir(dir);

	database.addDirectory(dir);
	database.addDirectory(dir2);

	database.addUser(std::make_shared<User>(user1));
	

	std::thread(handleConsoleCommand).detach();
	bool res = server->start();
	
	
	return 0;
}