#include <Utils/FileUtils.h>

#include <unistd.h>
#include <climits>

/**
 * Algorithm taken from: https://cplusplus.com/forum/general/11104/#msg52338
 */
std::string FileUtils::getGamePath() {
#ifdef NDEBUG
    char result[PATH_MAX];
    size_t length = readlink("/proc/self/exe", result, PATH_MAX);
    std::string dest = std::string( result, (length > 0) ? length : 0 );

    return dest.substr(0, dest.find_last_of("\\/"));
#else
    return "./";
#endif
}



std::string FileUtils::getWorkingDirectory() {
    char result[PATH_MAX];
    if (getcwd(result, PATH_MAX) != nullptr) return std::string(result);
    return "";
}