#ifndef FILEINFO_H
#define FILEINFO_H

#include <string>
#include <algorithm>

namespace trash
{
namespace utils
{

inline std::string fileDir(const std::string& filename)
{
    auto it = filename.find_last_of('/');
    if (it == std::string::npos)
        return "";
    return filename.substr(0, it+1);
}

inline std::string fileExt(const std::string& filename)
{
    auto it = filename.find_last_of('.');
    if (it == std::string::npos)
        return "";

    auto ext = filename.substr(it+1);
    for (auto& c: ext)
        c = static_cast<char>(tolower(c));

    return ext;
}

} // namespace
} // namespace

#endif // FILEINFO_H
