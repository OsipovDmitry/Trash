#include <core/trianglenode.h>

#include "nodeprivate.h"
#include "renderer.h"

class TriangleDrawable : public Drawable
{
public:
    TriangleDrawable(Node& node, std::shared_ptr<RenderProgram> rp, std::shared_ptr<Texture> dt, unsigned int v)
        : node_(node)
        , renderProgram_(rp)
        , diffuseTexture_(dt)
        , vao_(v)
    {
    }

    std::shared_ptr<RenderProgram> renderProgram() const { return renderProgram_; }
    std::shared_ptr<Texture> diffuseTexture() const { return diffuseTexture_; }
    glm::mat4x4 modelMatrix() const { return node_.globalTransform(); }

    GLuint vao() const { return vao_; }
    uint32_t numIndices() const { return 3; }

    Node& node_;
    std::shared_ptr<RenderProgram> renderProgram_;
    std::shared_ptr<Texture> diffuseTexture_;
    unsigned int vao_;
};

class TriangleNodePrivate : public NodePrivate
{
public:
    TriangleNodePrivate(Node& node)
        : NodePrivate(node)
    {}

    void doUpdate(Renderer& renderer, uint64_t time, uint64_t) override
    {
        if (!drawable)
        {
            std::shared_ptr<RenderProgram> renderProgram = renderer.loadRenderProgram(":/resources/shader.vert", ":/resources/shader.frag");
            std::shared_ptr<Texture> diffuseTexture = renderer.loadTexture(":/resources/brick.jpg");
            unsigned int vao = renderer.generateTriangle();
            drawable = std::make_shared<TriangleDrawable>(thisNode, renderProgram, diffuseTexture, vao);
        }
        renderer.draw(drawable);
    }

    std::shared_ptr<TriangleDrawable> drawable;
};

TriangleNode::TriangleNode()
    : Node(new TriangleNodePrivate(*this))
{
}
