#ifndef FILESYSTEM_ELEMENT_H
#define FILESYSTEM_ELEMENT_H

#include <string>
#include "Permission.h"
#include "Group.h"
#include "User.h"

class FilesystemElement {
public:
	std::string name;

	User* owner;
	Group* group;

	Permission userPermission;
	Permission groupPermission;
	Permission allPermission;

	FilesystemElement(const std::string& name, User* owner, Group* group, Permission userPermission, Permission groupPermission,
		Permission allPermission);

	bool hasPermission(User* user, Permission perm);
};

#endif // FILESYSTEM_ELEMENT_H