#ifndef NODE_H
#define NODE_H

#include <memory>

#include <utils/tree.h>
#include <utils/pimpl.h>
#include <utils/forwarddecl.h>

#include <core/coreglobal.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class NodeUserData
{
public:
    virtual ~NodeUserData() = default;
};

class DrawableNode;
class NodePrivate;

class CORESHARED_EXPORT Node : public utils::TreeNode<Node>
{
    PIMPL(Node)

public:
    Node();
    ~Node() override;

    virtual DrawableNode *asDrawableNode() { return nullptr; }
    virtual const DrawableNode *asDrawableNode() const { return nullptr; }

    void setTransform(const utils::Transform&);
    const utils::Transform& transform() const;
    const utils::Transform& globalTransform() const;

    const utils::BoundingBox& boundingBox() const;

    std::shared_ptr<NodeUserData> userData() const;
    void setUserData(std::shared_ptr<NodeUserData>);

protected:
    Node(NodePrivate*);

    void doAttach() override;
    void doDetach() override;

    std::unique_ptr<NodePrivate> m_; 
};

} // namespace
} // namespace

#endif // NODE_H
