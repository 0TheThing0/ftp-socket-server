#ifndef PERMISSION_H
#define PERMISSION_H
#include <string>

typedef enum Permission
{
	READ = 4,
	WRITE = 2,
	EXECUTE = 1,
	NOTHING = 0,
	ALL = READ | WRITE | EXECUTE,
	RW = READ | WRITE,
	RE = READ | EXECUTE,
	WE = WRITE | EXECUTE
} Permission;

const Permission defaultUser = ALL;
const Permission defaultGroup = ALL;
const Permission defaultAll = RE;

std::string parsePermission(Permission perm);

#endif // PERMISSION_H