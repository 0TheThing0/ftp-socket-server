#include "File.h"
#include <chrono>

std::string File::generateUniqueFilename() {
    // Получаем текущее время с точностью до миллисекунд
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    // Преобразуем в строку
    std::string filename = "file_" + std::to_string(milliseconds);
    return filename;
}

File::File(const std::string& name, const std::string& realPath, User* owner, Group* group, Permission userPermission, Permission groupPermission,
    Permission allPermission) : FilesystemElement(name,owner,group,userPermission,groupPermission,allPermission), realPath(realPath) {
    size = 1024;
}