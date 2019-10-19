#ifndef GRAPHICSCONTROLLERPRIVATE_H
#define GRAPHICSCONTROLLERPRIVATE_H

#include <memory>

#include <core/forwarddecl.h>

class Renderer;

class GraphicsControllerPrivate
{
public:
    Renderer& renderer;
    std::shared_ptr<Node> rootNode;

    GraphicsControllerPrivate(Renderer& r);
    void updateScene(uint64_t time, uint64_t dt);
};


#endif // GRAPHICSCONTROLLERPRIVATE_H
