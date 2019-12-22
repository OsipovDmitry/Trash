#ifndef RESOURCESTORAGE_H
#define RESOURCESTORAGE_H

#include <unordered_map>
#include <string>
#include <memory>

#include <utils/noncopyble.h>

namespace trash
{
namespace core
{

class ResourceStorage
{
    NONCOPYBLE(ResourceStorage)

public:
    class Object;

    ResourceStorage();
    ~ResourceStorage();

    void store(const std::string&, std::shared_ptr<Object>);
    std::shared_ptr<Object> get(const std::string&) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> m_storage;

};

class ResourceStorage::Object
{
    NONCOPYBLE(Object)
public:
    virtual ~Object() = default;

protected:
    Object() {}
};

} // namespace
} // namespace

#endif // RESOURCESTORAGE_H
