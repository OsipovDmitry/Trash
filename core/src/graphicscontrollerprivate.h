#ifndef GRAPHICSCONTROLLERPRIVATE_H
#define GRAPHICSCONTROLLERPRIVATE_H

#include <memory>
#include <unordered_map>

#include <glm/vec2.hpp>

#include <core/forwarddecl.h>

#include "abstractcontrollerprivate.h"

namespace trash
{
namespace core
{

class Renderer;

class GraphicsControllerPrivate : public AbstractControllerPrivate
{
public:
    std::shared_ptr<Scene> scene;
    glm::uvec2 currentViewportSize;

    GraphicsControllerPrivate();
    ~GraphicsControllerPrivate();

    void resize(int32_t, int32_t);
    void updateScene(uint64_t, uint64_t);
};

} // namespace
} // namespace

#endif // GRAPHICSCONTROLLERPRIVATE_H
