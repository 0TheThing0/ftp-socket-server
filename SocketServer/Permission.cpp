#include "Permission.h"

std::string parsePermission(Permission perm) {
	std::string ans = "---";
	if ((perm & READ) == READ)
		ans[0] = 'r';
	if ((perm & WRITE) == WRITE)
		ans[1] = 'w';
	if ((perm & EXECUTE) == EXECUTE)
		ans[2] = 'x';
	return ans;
}