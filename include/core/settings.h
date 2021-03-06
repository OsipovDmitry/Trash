#ifndef SETTINGS_H
#define SETTINGS_H

#include <memory>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <utils/noncopyble.h>
#include <utils/singletoon.h>
#include <utils/pimpl.h>

#include <core/coreglobal.h>

namespace trash
{
namespace core
{

class SettingsPrivate;

class CORESHARED_EXPORT Settings
{
    NONCOPYBLE(Settings)
    SINGLETON(Settings)
    PIMPL(Settings)

public:
    std::string readString(const std::string&, const std::string& = "");
    bool readBool(const std::string&, const bool = false);
    int32_t readInt32(const std::string&, const int32_t = 0);
    uint32_t readUint32(const std::string&, const uint32_t = 0u);
    float readFloat(const std::string&, const float = 0.f);
    glm::vec3 readVec3(const std::string&, const glm::vec3& = glm::vec3());
    glm::vec4 readVec4(const std::string&, const glm::vec4& = glm::vec4());

private:
    Settings();
    virtual ~Settings();

    std::unique_ptr<SettingsPrivate> m_;

};

} // namespace
} // namespace

#endif // SETTINGS_H
