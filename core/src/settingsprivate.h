#ifndef SETTINGSPRIVATE_H
#define SETTINGSPRIVATE_H

#include <string>

#include "rapidjson/document.h"

namespace trash
{
namespace core
{

class SettingsPrivate
{
public:
    SettingsPrivate();
    rapidjson::Document::ValueType *read(const std::string&);


    rapidjson::Document document;
};

} // namespace
} // namespace

#endif // SETTINGSPRIVATE_H
