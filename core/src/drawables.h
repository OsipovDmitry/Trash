#ifndef DRAWABLES_H
#define DRAWABLES_H

#include "renderer.h"

struct BoundingSphere;
struct Frustum;

class Drawable
{
public:
    virtual ~Drawable() = default;

    virtual std::shared_ptr<RenderProgram> renderProgram() const { return nullptr; }
    virtual std::shared_ptr<Mesh> mesh() const { return nullptr; }

    virtual void setup() {}
};

class MeshDrawable : public Drawable
{
public:
    MeshDrawable(std::shared_ptr<RenderProgram>, std::shared_ptr<Model::Mesh>, std::shared_ptr<VertexBuffer>);

    std::shared_ptr<RenderProgram> renderProgram() const override;
    std::shared_ptr<Mesh> mesh() const override;

    void setup() override;

    std::shared_ptr<RenderProgram> program;
    std::shared_ptr<Model::Mesh> mesh_;
    std::shared_ptr<VertexBuffer> animatedAttributesBuffer;
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

class FrustumDrawable : public Drawable
{
public:
    FrustumDrawable(std::shared_ptr<RenderProgram>, const Frustum&, const glm::vec4&);

    std::shared_ptr<RenderProgram> renderProgram() const override;
    std::shared_ptr<Mesh> mesh() const override;

    void setup() override;

    std::shared_ptr<RenderProgram> renderProgram_;
    std::shared_ptr<Mesh> mesh_;
    glm::vec4 color_;
};

#endif // DRAWABLES_H
