#ifndef GRAPHICSCONTROLLERPRIVATE_H
#define GRAPHICSCONTROLLERPRIVATE_H

#include <memory>
#include <unordered_map>

#include <glm/vec4.hpp>

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
    std::unordered_map<uint32_t, std::shared_ptr<Camera>> cameras;
    glm::ivec4 currentViewport;

    GraphicsControllerPrivate();
    ~GraphicsControllerPrivate();

    void resize(int32_t, int32_t);
    void updateScene(uint64_t, uint64_t);
};

} // namespace
} // namespace

#endif // GRAPHICSCONTROLLERPRIVATE_H
