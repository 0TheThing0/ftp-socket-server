#include "Directory.h"

Directory::Directory(const std::string& name, User* owner, Group* group, Permission userPermission, Permission groupPermission,
	Permission allPermission, std::shared_ptr<Directory>  parent) 
	: FilesystemElement(name, owner, group, userPermission, groupPermission, allPermission),
	parent(parent)
{
	//parent->addSubdirectory(std::make_shared<Directory>(this));
}

void Directory::addFile(const std::shared_ptr<File>& file) {
	files[file->name] = file;
}
void Directory::addSubdirectory(const std::shared_ptr<Directory>& dir) {
	subdirectories[dir->name] = dir;
}
std::shared_ptr<Directory> Directory::getSubdirectory(const std::string& name) {
	if (subdirectories.find(name) != subdirectories.end())
		return subdirectories[name];
	return nullptr;
}
std::shared_ptr<File> Directory::getFile(const std::string& name) {
	if (files.find(name) != files.end())
		return files[name];
	return nullptr;
}

void Directory::removeFile(const std::string& name) {
	files.erase(name);
}

void Directory::removeSubdirectory(const std::string& name) {
	subdirectories.erase(name);
}

std::string Directory::getPath() {
	return "/" + this->name;
}

std::string Directory::listContent() {
	std::string dirs;
	std::string files;

	for (const auto& element : this->subdirectories) {
		dirs.append(
			"d" +
			parsePermission(element.second->userPermission) +
			parsePermission(element.second->groupPermission) +
			parsePermission(element.second->allPermission) + " " +
			element.second->owner->username + " " +
			element.second->group->name + " " +
			"4096" + " " +
			"Apr 5 12:34" + " " +
			element.second->name + "/" +
			"\r\n");
	}

	for (const auto& element : this->files) {
		files.append(
			"-" +
			parsePermission(element.second->userPermission) +
			parsePermission(element.second->groupPermission) +
			parsePermission(element.second->allPermission) + " " +
			element.second->owner->username + " " +
			element.second->group->name + " " +
			std::to_string(element.second->size) + " " +
			"Apr 5 12:34" + " " +
			element.second->name +
			"\r\n");
	}
	return files + dirs;
}


std::string Directory::listContentName() {
	std::string dirs;
	std::string files;

	for (const auto& element : this->subdirectories) {
		dirs.append(
			element.second->name + "/" +
			" ");
	}

	for (const auto& element : this->files) {
		files.append(
			element.second->name +
			" ");
	}
	return files + dirs;
}
//"-rwxr-xr-x 1 user group 12345 Apr 5 12:34 file test.jar\r\ndrwxr-xr-x 2 user group 4096 Apr 5 12:34 directory/"