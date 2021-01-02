#include <QFile>

#include <core/settings.h>

#include "settingsprivate.h"

namespace trash
{
namespace core
{

std::string Settings::readString(const std::string &path, const std::string& defaultValue)
{
    auto obj = m_->read(path);
    return obj ? obj->GetString() : defaultValue;
}

bool Settings::readBool(const std::string& path, const bool defaultValue)
{
    auto obj = m_->read(path);
    return obj ? obj->GetBool() : defaultValue;
}

int32_t Settings::readInt32(const std::string& path, const int32_t defaultValue)
{
    auto obj = m_->read(path);
    return obj ? static_cast<int32_t>(obj->GetInt()) : defaultValue;
}

uint32_t Settings::readUint32(const std::string& path, const uint32_t defaultValue)
{
    auto obj = m_->read(path);
    return obj ? static_cast<uint32_t>(obj->GetUint()) : defaultValue;
}

float Settings::readFloat(const std::string& path, const float defaultValue)
{
    auto obj = m_->read(path);
    return obj ? obj->GetFloat() : defaultValue;
}

Settings::Settings()
    : m_(std::make_unique<SettingsPrivate>())
{
    QFile file("settings.json");
    if (!file.open(QFile::ReadOnly))
    {
        file.setFileName(QString(":/res/settings_default.json"));
        file.open(QFile::ReadOnly);
    }

    auto byteArray = file.readAll();
    file.close();

    m_->document.Parse(byteArray);
}

Settings::~Settings()
{
}

} // namespace
} // namespace
