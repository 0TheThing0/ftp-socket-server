#include "Group.h"

void Group::addUser(User* user) {
	users[user->username] = user;
	user->addGroup(this);
}

bool Group::isMember(User* user) {
	return users.find(user->username) != users.end();
}