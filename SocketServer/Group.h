#ifndef GROUP_H
#define GROUP_H
#include "User.h"
#include <string>
#include <unordered_map>

class User;

class Group {
public:
	std::string name;
	std::unordered_map<std::string,User*> users;

	Group(const std::string& name) : name(name) {}

	void addUser(User* user);
	bool isMember(User* user);
private:
	friend class User;
};

#endif // GROUP_H