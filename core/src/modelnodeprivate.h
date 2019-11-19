#ifndef MODELNODEPRIVATE_H
#define MODELNODEPRIVATE_H

#include <string>
#include <vector>

#include "nodeprivate.h"

struct RenderProgram;
struct Model;
struct Buffer;
struct VertexBuffer;
class MeshDrawable;

class ModelNodePrivate : public NodePrivate
{
public:
    ModelNodePrivate(Node&, const std::string&);

    void doUpdate(uint64_t, uint64_t) override;
    BoundingSphere calcLocalBoundingSphere() override;

    std::string modelName;
    std::shared_ptr<Model> model;
    std::shared_ptr<RenderProgram> renderProgram, animatedAttributesProgram;
    std::shared_ptr<Buffer> bonesBuffer;
    std::vector<std::pair<std::shared_ptr<MeshDrawable>, Transform>> meshDrawables;
    std::vector<std::shared_ptr<VertexBuffer>> animatedAttributes;
    std::string animationName;
    uint32_t timeOffset;
    bool showBones;
};

#endif // MODELNODEPRIVATE_H
