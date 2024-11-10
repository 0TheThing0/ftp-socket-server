#include "User.h"

void User::addGroup(Group* group) {
	groups.insert(group);
}

bool User::belongsToGroup(Group* group) {
	return groups.find(group) != groups.end();
}

void User::setRootDir(std::shared_ptr<Directory> dir) {
	this->rootDir = dir;
}