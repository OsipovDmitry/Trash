#ifndef GRAPHICSCONTROLLERPRIVATE_H
#define GRAPHICSCONTROLLERPRIVATE_H

#include <memory>

#include <glm/mat4x4.hpp>

#include <core/forwarddecl.h>

#include "abstractcontrollerprivate.h"

class Renderer;

class GraphicsControllerPrivate : public AbstractControllerPrivate
{
public:
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<AbstractCamera> camera;
    glm::mat4x4 viewMatrix;

    GraphicsControllerPrivate();
    ~GraphicsControllerPrivate();

    void updateScene(uint64_t, uint64_t);
    std::shared_ptr<Node> pickNode(int32_t, int32_t);
};


#endif // GRAPHICSCONTROLLERPRIVATE_H
