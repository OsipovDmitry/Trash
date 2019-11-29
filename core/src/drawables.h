#ifndef DRAWABLES_H
#define DRAWABLES_H

#include "renderer.h"

struct BoundingSphere;
struct Frustum;

class SelectionDrawable;

class Drawable
{
public:
    virtual ~Drawable() = default;

    virtual LayerId layerId() const = 0;
    virtual std::shared_ptr<RenderProgram> renderProgram() const = 0;
    virtual std::shared_ptr<Mesh> mesh() const = 0;
    virtual std::shared_ptr<SelectionDrawable> selectionDrawable(uint32_t) const = 0;

    virtual void prerender() const {}
    virtual void postrender() const {}
};

class SelectionDrawable : public Drawable
{
public:
    SelectionDrawable(uint32_t);

    LayerId layerId() const override { return LayerId::Selection; }

    uint32_t id;

    static uint32_t colorToId(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    static glm::vec4 idToColor(uint32_t id);
};

class MeshDrawable : public Drawable
{
public:
    MeshDrawable(std::shared_ptr<RenderProgram>, std::shared_ptr<Mesh>, std::shared_ptr<Buffer>);

    LayerId layerId() const override { return LayerId::SolidGeometry; }
    std::shared_ptr<RenderProgram> renderProgram() const override;
    std::shared_ptr<Mesh> mesh() const override;
    std::shared_ptr<SelectionDrawable> selectionDrawable(uint32_t) const override;

    void prerender() const override;

    std::shared_ptr<RenderProgram> program;
    std::shared_ptr<Mesh> geometry;
    std::shared_ptr<Buffer> bonesBuffer;
};

class SelectionMeshDrawable : public SelectionDrawable
{
public:
    SelectionMeshDrawable(std::shared_ptr<Mesh>, std::shared_ptr<Buffer>, uint32_t);

    std::shared_ptr<RenderProgram> renderProgram() const override;
    std::shared_ptr<Mesh> mesh() const override;
    std::shared_ptr<SelectionDrawable> selectionDrawable(uint32_t) const override;

    void prerender() const override;

    std::shared_ptr<RenderProgram> program;
    std::shared_ptr<Mesh> geometry;
    std::shared_ptr<Buffer> bonesBuffer;
};

class ColoredMeshDrawable : public MeshDrawable
{
public:
    ColoredMeshDrawable(std::shared_ptr<Mesh>, const glm::vec4&, std::shared_ptr<Buffer>);

    void prerender() const override;

    glm::vec4 color;
};

class TexturedMeshDrawable : public MeshDrawable
{
public:
    TexturedMeshDrawable(std::shared_ptr<Mesh>, std::shared_ptr<Texture>, std::shared_ptr<Buffer>);

    void prerender() const override;

    std::shared_ptr<Texture> diffuseTexture;
};

class SphereDrawable : public ColoredMeshDrawable
{
public:
    SphereDrawable(uint32_t, const BoundingSphere&, const glm::vec4&);
};

class FrustumDrawable : public ColoredMeshDrawable
{
public:
    FrustumDrawable(const Frustum&, const glm::vec4&);
};

#endif // DRAWABLES_H
