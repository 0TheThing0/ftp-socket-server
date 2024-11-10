#ifndef FILE_H
#define FILE_H

#include <string>
#include "FilesystemElement.h"
#include "User.h"
#include "Group.h"

class File : public FilesystemElement {
public:
	size_t size;
	std::string realPath;

	File(const std::string& name, const std::string& realPath, User* owner, Group* group, Permission userPermission, Permission groupPermission,
		Permission allPermission);
	static std::string generateUniqueFilename();
};

#endif // FILE_H