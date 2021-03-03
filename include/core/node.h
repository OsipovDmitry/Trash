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

class NodePrivate;

class CORESHARED_EXPORT Node : public utils::TreeNode<Node>, public std::enable_shared_from_this<Node>
{
    PIMPL(Node)

public:
    Node();
    ~Node() override;

    void setTransform(const utils::Transform&);
    const utils::Transform& transform() const;
    const utils::Transform& globalTransform() const;

    const utils::BoundingBox& boundingBox() const;

    void accept(NodeVisitor&);

    std::shared_ptr<const NodeUserData> userData() const;
    std::shared_ptr<NodeUserData> userData();
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
