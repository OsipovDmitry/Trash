#include <assert.h>

#include "resourcestorage.h"
#include "renderer.h"

ResourceStorage::ResourceStorage()
{
}

ResourceStorage::~ResourceStorage()
{
}

void ResourceStorage::store(const std::string& key, std::shared_ptr<ResourceStorage::Object> value)
{
    auto iter = m_storage.find(key);

    if (iter != m_storage.end())
        assert(iter->second == value);

    m_storage[key] = value;
}

std::shared_ptr<ResourceStorage::Object> ResourceStorage::get(const std::string& key) const
{
    auto iter = m_storage.find(key);

    if (iter == m_storage.end())
        return nullptr;

    return iter->second;
}
