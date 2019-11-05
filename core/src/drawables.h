#ifndef DRAWABLES_H
#define DRAWABLES_H

#include "renderer.h"

class BoundingSphere;

class Drawable
{
public:
    virtual ~Drawable() = default;

    virtual std::shared_ptr<RenderProgram> renderProgram() const { return nullptr; }
    virtual std::shared_ptr<Mesh> mesh() const { return nullptr; }

    virtual void setup() {}
};

class SkeletalMeshDrawable : public Drawable
{
public:
    SkeletalMeshDrawable(std::shared_ptr<RenderProgram>, std::shared_ptr<Model::Mesh>, std::shared_ptr<UniformBuffer>);

    std::shared_ptr<RenderProgram> renderProgram() const override;
    std::shared_ptr<Mesh> mesh() const override;

    void setup() override;

    std::shared_ptr<RenderProgram> program;
    std::shared_ptr<Model::Mesh> skeletalMesh;
    std::shared_ptr<UniformBuffer> bufferData;
};

class SphereDrawable : public Drawable
{
public:
    SphereDrawable(std::shared_ptr<RenderProgram>, uint32_t, const BoundingSphere&, const glm::vec4&);

    std::shared_ptr<RenderProgram> renderProgram() const override;
    std::shared_ptr<Mesh> mesh() const override;

    void setup() override;

    std::shared_ptr<RenderProgram> renderProgram_;
    std::shared_ptr<Mesh> mesh_;
    glm::vec4 color_;
};

#endif // DRAWABLES_H
