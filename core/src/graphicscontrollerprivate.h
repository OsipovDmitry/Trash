#ifndef GRAPHICSCONTROLLERPRIVATE_H
#define GRAPHICSCONTROLLERPRIVATE_H

#include <memory>

#include <core/forwarddecl.h>

#include "abstractcontrollerprivate.h"

class Renderer;

class GraphicsControllerPrivate : public AbstractControllerPrivate
{
public:
    Renderer& renderer;
    std::shared_ptr<Node> rootNode;

    GraphicsControllerPrivate(Renderer&);
    ~GraphicsControllerPrivate();
    void updateScene(uint64_t time, uint64_t dt);
};


#endif // GRAPHICSCONTROLLERPRIVATE_H
