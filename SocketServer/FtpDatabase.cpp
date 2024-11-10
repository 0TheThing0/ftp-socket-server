#include "FtpDatabase.h"

bool FtpDatabase::addUser(std::shared_ptr<User> user) {
	if (users.find(user->username) != users.end())
		return false;
	users[user->username] = user;
	return true;
}
std::shared_ptr<User> FtpDatabase::getUser(const std::string& username) {
	if (users.find(username) != users.end()) {
		return users[username];
	}
	return NULL;
}

bool FtpDatabase::addGroup(std::shared_ptr<Group> group) {
	if (groups.find(group->name) != groups.end())
		return false;
	groups[group->name] = group;
	return true;
}
std::shared_ptr<Group> FtpDatabase::getGroup(const std::string& name) {
	if (groups.find(name) != groups.end()) {
		return groups[name];
	}
	return NULL;
}

bool FtpDatabase::addFile(std::shared_ptr<File> file) {
	files.push_back(file);
	return true;
}
bool FtpDatabase::addDirectory(std::shared_ptr<Directory> dir) {
	directories.push_back(dir);
	return true;
}
