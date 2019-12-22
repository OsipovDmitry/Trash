#include <core/light.h>

#include "sceneprivate.h"
#include "lightprivate.h"


namespace trash
{
namespace core
{

void ScenePrivate::dirtyLight(Light *light)
{
    if (light == nullptr)
        dirtyAllLights = true;
    else
        dirtyLights.insert(light);
}

std::shared_ptr<Buffer> ScenePrivate::getLightsBuffer()
{
    if (dirtyAllLights || !dirtyLights.empty())
    {
        if (dirtyAllLights)
        {
            size_t bufferSize = sizeof(glm::mat4x4) * lights.size();
            lightsUbo = std::make_shared<Buffer>(bufferSize, nullptr, GL_STATIC_DRAW);
            auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(0, static_cast<GLsizeiptr>(bufferSize), GL_MAP_WRITE_BIT));
            for (size_t i = 0; i < lights.size(); ++i)
                p[i] = lights[i]->m().pack();
            lightsUbo->unmap();
        }
        else
        {

        }

        dirtyAllLights = false;
        dirtyLights.clear();
    }

    return lightsUbo;
}


} // namespace
} // namespace
