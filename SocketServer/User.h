#ifndef USER_H
#define USER_H

#include <string>
#include <unordered_set>
#include <memory>
#include "Group.h"

class Directory;
class Group;

class User {
public:
	std::string username;
	std::string password;
	std::unordered_set<Group*> groups;
	Group* defaultGroup;
	std::shared_ptr<Directory> rootDir;
	User(const std::string& username, const std::string& password, Group* group) : username(username), password(password){
		rootDir = NULL;
		defaultGroup = group;
		groups.insert(group);
	}


	bool belongsToGroup(Group* group);
	void setRootDir(std::shared_ptr<Directory> dir);

private:
	void addGroup(Group* group);
	friend class Group;
};

#endif // USER_H