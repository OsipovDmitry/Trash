#ifndef MODELNODEPRIVATE_H
#define MODELNODEPRIVATE_H

#include <string>

#include "nodeprivate.h"

struct RenderProgram;
struct Model;

class ModelNodePrivate : public NodePrivate
{
public:
    ModelNodePrivate(Node&, const std::string&);

    void doUpdate(Renderer&, uint64_t, uint64_t) override;

    std::string modelName;
    std::shared_ptr<Model> model;
    std::shared_ptr<RenderProgram> renderProgram;
    std::string animationName;
    uint32_t timeOffset;
};

#endif // MODELNODEPRIVATE_H
