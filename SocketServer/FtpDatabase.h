#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <list>
#include "Permission.h"
#include "User.h"
#include "Directory.h"
#include "File.h"
#include "Group.h"

class FtpDatabase
{
public:
	bool addUser(std::shared_ptr<User> user);
	std::shared_ptr<User> getUser(const std::string& username);

	bool addGroup(std::shared_ptr<Group> group);
	std::shared_ptr<Group> getGroup(const std::string& name);

	bool addFile(std::shared_ptr<File> file);
	bool addDirectory(std::shared_ptr<Directory> dir);

private:
	std::unordered_map<std::string, std::shared_ptr<User>> users;
	std::unordered_map<std::string, std::shared_ptr<Group>> groups;
	std::list<std::shared_ptr<Directory>> directories;
	std::list<std::shared_ptr<File>> files;
};
