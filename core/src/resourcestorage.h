#ifndef RESOURCESTORAGE_H
#define RESOURCESTORAGE_H

#include <unordered_map>
#include <string>
#include <memory>

#include <utils/noncopyble.h>
#include <utils/singletoon.h>

class ResourceStorage
{
    SINGLETON(ResourceStorage)
    NONCOPYBLE(ResourceStorage)

public:
    class Object;

    void store(const std::string&, std::shared_ptr<Object>);
    std::shared_ptr<Object> get(const std::string&) const;

private:
    ResourceStorage();
    ~ResourceStorage() = default;

    std::unordered_map<std::string, std::weak_ptr<Object>> m_storage;
};

class ResourceStorage::Object
{
    NONCOPYBLE(Object)
public:
    virtual ~Object() = default;

protected:
    Object() {}
};

#endif // RESOURCESTORAGE_H
