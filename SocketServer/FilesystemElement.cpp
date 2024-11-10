#include "FilesystemElement.h"


FilesystemElement::FilesystemElement(
	const std::string& name, User* owner, Group* group,
	Permission userPermission, Permission groupPermission,
	Permission allPermission)
	:name(name), owner(owner), group(group), userPermission(userPermission),
	groupPermission(groupPermission), allPermission(allPermission) {}


bool FilesystemElement::hasPermission(User* user, Permission perm) {
	if (user->username == owner->username) {
		return (userPermission & perm) != 0;
	}

	if (group->isMember(user)) {
		return (groupPermission & perm) != 0;
	}

	return (allPermission & perm) != 0;
}