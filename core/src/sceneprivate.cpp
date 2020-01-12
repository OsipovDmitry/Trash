#include <algorithm>

#include <core/light.h>

#include "renderer.h"
#include "sceneprivate.h"
#include "lightprivate.h"


namespace trash
{
namespace core
{

SceneRootNode::SceneRootNode(Scene *scene)
    : Node(new NodePrivate(*this))
    , m_scene(scene)
{
}

Scene *SceneRootNode::scene() const
{
    return m_scene;
}

void ScenePrivate::dirtyLight(Light *light)
{
    if (light == nullptr)
        dirtyAllLights = true;
    else
        dirtyLights.insert(std::distance(lights.begin(),
                                         std::find_if(lights.begin(), lights.end(), [light](const std::shared_ptr<trash::core::Light>& l){ return l.get() == light; })));
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
            for (auto lightIdx : dirtyLights)
            {
                auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(static_cast<GLintptr>(lightIdx * sizeof(glm::mat4x4)), static_cast<GLsizeiptr>(sizeof(glm::mat4x4)), GL_MAP_WRITE_BIT));
                *p = lights[static_cast<size_t>(lightIdx)]->m().pack();
                lightsUbo->unmap();
            }
        }

        dirtyAllLights = false;
        dirtyLights.clear();
    }

    return lightsUbo;
}

} // namespace
} // namespace
