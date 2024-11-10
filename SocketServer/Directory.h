#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "FilesystemElement.h"
#include "File.h"
#include "User.h"
#include "Group.h"
#include <unordered_map>
#include <memory>

class Directory : public FilesystemElement{
public:
	std::unordered_map<std::string, std::shared_ptr<File>> files;   // Файлы в каталоге
	std::unordered_map<std::string, std::shared_ptr<Directory>> subdirectories; // Подкаталоги

	std::shared_ptr<Directory> parent;


	Directory(const std::string& name, User* owner, Group* group, Permission userPermission, Permission groupPermission,
		Permission allPermission, std::shared_ptr<Directory> parent = nullptr);

	void addFile(const std::shared_ptr<File>& file);
	void addSubdirectory(const std::shared_ptr<Directory>& dir);
	
	std::string getPath();
	std::string listContent();
	std::string listContentName();

	std::shared_ptr<Directory> getSubdirectory(const std::string& name);
	std::shared_ptr<File> getFile(const std::string& name);
	
	void removeFile(const std::string& name);
	void removeSubdirectory(const std::string& name);
};

#endif // DIRECTORY_H