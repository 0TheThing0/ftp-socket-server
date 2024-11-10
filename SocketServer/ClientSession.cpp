#include "ClientSession.h"
#include <memory>
#include <sstream>

std::mutex coutMutex;

void safePrint(const std::string& str) {
	std::lock_guard<std::mutex> lock(coutMutex);
	std::cout << str << std::endl;
}


void ClientSession::handleCommandSocket() {
	char* buffer = (char*)malloc(4096 * sizeof(CHAR));
	std::string welcomeMessage = "220 Welcome to My FTP Server\r\n";
	send(this->commandSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);

	std::thread connectionThread;
	std::string portStr = std::to_string(this->clientPort);

	std::shared_ptr<FilesystemElement> elementToRename = nullptr;

	safePrint(">>>>>New connection from " + this->clientAddress + ":" + portStr + "<<<<<");
	while (this->server->serverRunning) {
		memset(buffer, 0, 4096);
		int bytesRead = recv(this->commandSocket, buffer, 4096 - 1, 0);
		if (bytesRead <= 0) break; // Ошибка или закрыто соединение


		std::string command(buffer);

		safePrint(">>>>Received: " + command + "from " + this->clientAddress + ":" + portStr);

		std::string response;
		
		//Logging part
		if (strncmp(buffer, "USER", 4) == 0) {
			std::string username = command.substr(5);
			username.erase(username.end() - 2, username.end());

			this->user = this->server->database->getUser(username);
			if (this->user == NULL)
				response = "430 Invalid username.\r\n";
			else
				response = "331 User name okay, need password.\r\n";
		}
		else if (strncmp(buffer, "PASS", 4) == 0) {
			std::string password = command.substr(5);
			password.erase(password.end() - 2, password.end());
			if (this->user != NULL && this->user->password.compare(password) == 0) {
				response = "230 User logged in, proceed.\r\n";
				this->currentDirectory = user->rootDir;
				this->isAuthorized = true;
			}
			else
				response = "430 Invalid username or password.\r\n";
		}

		//Block everything if unauthorized
		else if (!this->isAuthorized) {
			response = "530 Not logged in.\r\n";
		}

		//Rename from
		else if (strncmp(buffer, "RNFR", 4) == 0) {
			std::string filename = command.substr(5);
			filename.erase(filename.end() - 2, filename.end());
			
			elementToRename = this->currentDirectory->getFile(filename);
			if (elementToRename == nullptr) {
				elementToRename = this->currentDirectory->getSubdirectory(filename);
			}

			if (elementToRename != nullptr && elementToRename->hasPermission(this->user.get(),WRITE)) {
				response = "350 Ready for RNTO.\r\n";
			}
			else {
				response = "550 File not found or permission denied.\r\n";
			}
		}

		//Rename to
		else if (strncmp(buffer, "RNTO", 4) == 0) {
			std::string filename = command.substr(5);
			filename.erase(filename.end() - 2, filename.end());

			if (elementToRename != nullptr) {
				if (currentDirectory->getFile(filename) == nullptr && currentDirectory->getSubdirectory(filename) == nullptr) {
					elementToRename->name = filename;
					response = "250 Rename successful.\r\n";
				}
				else {
					response = "550 Element with such name already exists.\r\n";
				}
			}
			else {
				response = "550 Call RNFR first.\r\n";
			}
			elementToRename = nullptr;
		}

		//Change directory
		else if (strncmp(buffer, "CWD", 3) == 0) {

			//Getting path to new directory
			std::string newDir = command.substr(4);
			newDir.erase(newDir.end() - 2, newDir.end());
			

			char* context = NULL;
			//Is it absolute or relative path?

			//Absolute
			if (newDir[0] == '/') {
				this->currentDirectory = this->user->rootDir;
			}

			char* path = strtok_s(&newDir[0], "/",&context);


			std::shared_ptr<Directory> subDir = this->currentDirectory;
			while (path != NULL)  
			{
				subDir = subDir->getSubdirectory(path);

				//Check is directory avaliable to open
				if (subDir == NULL || !subDir->hasPermission(this->user.get(), READ)) {
					break;
				}

				path = strtok_s(NULL, "/",&context);
			}

			if (subDir == NULL) {
				response = "430 Invalid directory path\r\n";
			}
			else {
				this->currentDirectory = subDir;
				response = "250 Directory changed to " + subDir->getPath() + "\r\n";
			}
		}


		//Get current directory absolute path
		else if (strncmp(buffer, "PWD", 3) == 0) {
			response = "257 \"" + this->createPath() + "\" is the current directory.\r\n";
		}

		//Change to parent directory
		else if (strncmp(buffer, "CDUP", 4) == 0) {

			if (this->currentDirectory->parent != nullptr 
				&& this->currentDirectory->parent->hasPermission(this->user.get(),READ)) {
				this->currentDirectory = this->currentDirectory->parent;
				response = "250 CWD command successful.\r\n";
			}
			else
				response = "550 Failed to change directory.\r\n";
		}

		//Delete file from server
		else if (strncmp(buffer, "DELE", 4) == 0) {
			std::string filename = command.substr(5);
			filename.erase(filename.end() - 2, filename.end());


			std::shared_ptr<File> file = this->currentDirectory->getFile(filename);
			if (file == NULL) {
				response = "550 File not found.\r\n";
			}
			else {
				if (file->hasPermission(this->user.get(), WRITE)) {
					this->currentDirectory->removeFile(filename);
					response = "250 File deleted successfully.\r\n";
				} 
				else {
					response = "550 Permission denied.\r\n";
				}
			}
		}

		//Enter a passive mode to transfer data
		else if (strncmp(buffer, "PASV", 4) == 0) {
			bool res = createPassiveSocket();

			if (res) {
		
				SOCKADDR_IN addr;
				int len = sizeof(addr);
				getsockname(dataSocket, (struct sockaddr*)&addr, &len);
				int port = ntohs(addr.sin_port);
				response = "227 Entering Passive Mode ("
					+ std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + ','
					+ std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + ','
					+ std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + ','
					+ std::to_string(addr.sin_addr.S_un.S_un_b.s_b4) + ','
					+ std::to_string(port / 256) + ','
					+ std::to_string(port % 256) + ").\r\n";
				//Set thread to wait new connection
				connectionThread = std::thread(&ClientSession::handleDataSocket, this);
			}
			else
				response = "425 Can't open data connection.\r\n";
		}

		//List all elements in directory (WITHOUT path)
		else if (strncmp(buffer, "LIST", 4) == 0) {
			connectionThread.join();

			if (this->currentDirectory->hasPermission(this->user.get(), READ))
			response = "150 Here comes the directory listing.\r\n";
			send(this->commandSocket, response.c_str(), response.size(), 0);
			sendDirectoryList();
			response = "226 Directoty send OK.\r\n";
		}

		//List all elements names in directory
		else if (strncmp(buffer, "NLST", 4) == 0) {
			connectionThread.join();
			response = "150 Here comes the directory listing.\r\n";
			send(this->commandSocket, response.c_str(), response.size(), 0);
			sendDirectoryNameList();
			response = "226 Directoty send OK.\r\n";
		}

		//Send file to client
		else if (strncmp(buffer, "RETR", 4) == 0) {
			std::string filepath = command.substr(5);
			filepath.erase(filepath.end() - 2, filepath.end());

			char* context = NULL;

			char* path = strtok_s(&filepath[0], "/", &context);
			std::string filename;


			std::shared_ptr<Directory> subDir;
			while (path != NULL)
			{
				filename = path;
				path = strtok_s(NULL, "/", &context);
			}

			connectionThread.join();
			
			std::shared_ptr<File> file = this->currentDirectory->getFile(filename);
			if (file == NULL) {
				response = "550 File not found.\r\n";
			}
			else if (!file->hasPermission(this->user.get(), READ)) {
				response = "550 Permission denied\r\n";
			}
			else {
				response = "150 Opening connection for " + filename + ".\r\n";
				send(this->commandSocket, response.c_str(), response.size(), 0);
				if (this->transferType == NONE) {
					response = "550 transfer type unspecified.\r\n";
				}
				else {
					sendFileData(file->realPath);
					response = "226 Transfer complete.\r\n";
				}
			}

		}

		//Load file to server
		else if (strncmp(buffer, "STOR", 4) == 0) {
			std::string filename = command.substr(5);
			filename.erase(filename.end() - 2, filename.end());
			connectionThread.join();
			
			std::shared_ptr<File> file = this->currentDirectory->getFile(filename);

			//Check that file avaliable to change or create
			if ((file==nullptr && this->currentDirectory->hasPermission(this->user.get(),WRITE))
				|| (file != nullptr && file->hasPermission(this->user.get(), WRITE))) {
				if (file == nullptr) {
					const std::string uniqueName = File::generateUniqueFilename();

					file = std::shared_ptr<File>(std::make_shared<File>(filename, this->server->filesStorage + uniqueName,
						this->user.get(), this->user->defaultGroup, defaultUser, defaultGroup, defaultAll));
					this->currentDirectory->addFile(file);
				}
				response = "150 Opening connection for receiving " + filename + ".\r\n";
				send(this->commandSocket, response.c_str(), response.size(), 0);
				getFileData(file->realPath);
				response = "226 Transfer complete.\r\n";
			}
			else {
				response = "550 Permission denied.\r\n";
			}
		}

		//Store file with unique name
		else if (strncmp(buffer, "STOU", 4) == 0) {
			const std::string uniqueName = File::generateUniqueFilename();
			std::shared_ptr<File> file(std::make_shared<File>(uniqueName, this->server->filesStorage + uniqueName,
				this->user.get(), this->user->defaultGroup, defaultUser, defaultGroup, defaultAll));
				response = "150 Opening connection for unique file storage.\r\n";
				send(this->commandSocket, response.c_str(), response.size(), 0);
				getFileData(file->realPath);
				response = "226 File stored as " + file->name + ".\r\n";
				this->currentDirectory->addFile(file);
		}

		//Create directory
		else if (strncmp(buffer, "MKD", 3) == 0) {
			std::string path = command.substr(4); // Извлечение новой директории
			path.erase(path.end() - 2, path.end());

			if (this->currentDirectory->hasPermission(this->user.get(), WRITE)) {
				if (this->currentDirectory->subdirectories.find(path) != this->currentDirectory->subdirectories.end()) {
					response = "550 Directory already exists.\r\n";
				}
				else {
					const std::string dirName = path;
					Directory newDir(path, this->user.get(), (this->user->defaultGroup),
						defaultUser, defaultGroup, defaultAll, this->currentDirectory);
					this->currentDirectory->addSubdirectory(std::make_shared<Directory>(newDir));
					response = "257 \"" + path + "\" directory created.\r\n";
				}
			}
			else
				response = "550 Permission denied.\r\n";
		}

		//Delete directory
		else if (strncmp(buffer, "RMD", 3) == 0) {
			std::string path = command.substr(4); // Извлечение новой директории
			path.erase(path.end() - 2, path.end());

			std::shared_ptr<Directory> dir = this->currentDirectory->getSubdirectory(path);
			if (dir == NULL) {
				response = "550 Directory not found.\r\n";
			}
			else {
				if (!dir->hasPermission(this->user.get(), WRITE)) {
					response = "550 Permission denied.\r\n";
				}
				else if (!dir->subdirectories.empty() || !dir->files.empty()) {
					response = "550 Directory not empty.\r\n";
				}
				else {
					this->currentDirectory->removeSubdirectory(path);
					response = "250 Directory deleted.\r\n";
				}
			}
		}

		//Exit without losing connection
		else if (strncmp(buffer, "REIN", 4) == 0) {
			this->user = NULL;
			this->isAuthorized = false;
			response = "220 Service ready for new user.";
		}

		//Set transfer type
		else if (strncmp(buffer, "TYPE", 4) == 0) {
			std::string type = command.substr(5);
			type.erase(type.end() - 2, type.end());
			if (type.compare("I") == 0) {
				this->transferType = BINARY;
				response = "200 Type set to " + type + ".\r\n";
			}
			else if (type.compare("A") == 0) {
				this->transferType = ASCII;
				response = "200 Type set to " + type + ".\r\n";
			}
			else
			{
				response = "504 Command not implemented for that parameter.\r\n";
			}
		}

		//Quit from server
		else if (strncmp(buffer, "QUIT", 4) == 0) {
			response = "221 Goodbye.\r\n";
			send(this->commandSocket, response.c_str(), response.size(), 0);
			safePrint("<<<<Answer: " + response + "to " + this->clientAddress + ":" + portStr);
			break;
		}
		else if (strncmp(buffer, "NOOP", 4) == 0) {
			response = "200 NOOP command successful.\r\n";
		}

		else {
			response = "502 Command not implemented.\r\n";
		}

		send(this->commandSocket, response.c_str(), response.size(), 0);
		safePrint("<<<<Answer: " + response + "to " + this->clientAddress + ":" + portStr);
	
		if ((strncmp(buffer, "RNFR", 4) != 0)) {
			elementToRename = nullptr;
		}
}
	// Удаление сессии
	{
		//std::lock_guard<std::mutex> lock(sessionMutex);
		this->server->sessions.erase(this->commandSocket);
	}
	closesocket(this->commandSocket);
}


std::string ClientSession::createPath() {
	std::shared_ptr<Directory> curDir = this->currentDirectory;

	std::vector<std::string> pathParts;

	// Проходим по цепочке родителей, начиная с текущей директории
	while (curDir != nullptr && curDir != this->user->rootDir) {
		pathParts.push_back(curDir->name);
		curDir = curDir->parent;
	}

	// Строим путь от корня к текущей директории (переворачиваем)
	std::ostringstream fullPath;
	for (auto it = pathParts.rbegin(); it != pathParts.rend(); ++it) {
	
		fullPath << "/" << *it;
	}
	std::string res = fullPath.str();
	if (res.compare("") == 0)
		return "/";
	else
		return fullPath.str();
}

bool ClientSession::createPassiveSocket() {
	SOCKADDR_IN addr;

	int len = sizeof(addr);
	int iResult = getsockname(commandSocket, (struct sockaddr*)&addr, &len);
	if (iResult != 0)
		return false;

	
	addr.sin_port = 0;
	
	
	dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (dataSocket == INVALID_SOCKET) {
		return false;
	}

	iResult = bind(dataSocket, (struct sockaddr*)&addr, len);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(dataSocket);
		return false;
	}
	
	iResult = getsockname(dataSocket, (struct sockaddr*)&addr, &len);

	if (listen(dataSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(dataSocket);
		return false;
	}
	return true;
}

void ClientSession::handleDataSocket() {
	char* buffer = (char*)malloc(4096 * sizeof(CHAR));
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	while (true) {

		SOCKET clientSocket = accept(dataSocket, (struct sockaddr*)&clientAddr, &addrLen);

		if (clientSocket == INVALID_SOCKET) {
			safePrint("Error accepting connection. " + WSAGetLastError());
			return;
		}


		char connectedAddress[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, connectedAddress, sizeof(connectedAddress));

		safePrint("New connection attempt to data port from " + std::string(connectedAddress) + ":" + std::to_string(clientAddr.sin_port));
		if (this->clientAddress.compare(connectedAddress) == 0) {
			this->userSocket = clientSocket;
			safePrint("Connetion accepted");
			break;
		}
		else {
			safePrint("Connetion refused");
		}
		//closesocket(clientSocket);
	}
	closesocket(this->dataSocket);
	this->dataSocket = INVALID_SOCKET;
}

void ClientSession::sendDirectoryList() {

	std::string res = this->currentDirectory->listContent();
	send(this->userSocket, res.c_str(), res.length(), 0);
	closesocket(this->userSocket);
	this->userSocket = INVALID_SOCKET;
}

void ClientSession::sendDirectoryNameList() {
	std::string res = this->currentDirectory->listContentName();
	send(this->userSocket, res.c_str(), res.length(), 0);
	closesocket(this->userSocket);
	this->userSocket = INVALID_SOCKET;
}

void ClientSession::sendFileData(std::string& filePath) {
	std::ifstream inputFile;
	if (this->transferType == BINARY) {
		inputFile = std::ifstream(filePath, std::ios::binary);
	}
	else if (this->transferType == ASCII) {
		inputFile = std::ifstream(filePath, std::ios::in);
	}
	else
		return;
	if (!inputFile) {
		std::cout << "Cannot open file!" << std::endl;
		return;
	}

	char buffer[FILE_BUFFER_SIZE];
	while (!inputFile.eof()) {
		inputFile.read(buffer, FILE_BUFFER_SIZE);
		size_t bytesToSend = inputFile.gcount();
		if (send(this->userSocket, buffer, bytesToSend, 0) == -1) {
			std::cout << "Sending error!" << std::endl;
			break;
		}
	}
	inputFile.close();

	closesocket(this->userSocket);
	this->userSocket = INVALID_SOCKET;
}

void ClientSession::getFileData(std::string& downloadPath) {
	char buffer[FILE_BUFFER_SIZE];

	std::ofstream outputFile(downloadPath, std::ios::binary);

	if (!outputFile) {
		std::cout << "Cannot open file" << std::endl;
		return;
	}

	int bytesReceived;
	while ((bytesReceived = recv(this->userSocket, buffer, FILE_BUFFER_SIZE, 0)) > 0) {
		outputFile.write(buffer, bytesReceived);
	}

	if (bytesReceived < 0) {
		std::cout << "Receiving error!" << std::endl;
	}
	
	outputFile.close();
	closesocket(this->userSocket);
	this->userSocket = INVALID_SOCKET;
}