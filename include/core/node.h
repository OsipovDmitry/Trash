#ifndef NODE_H
#define NODE_H

#include <memory>

#include <utils/tree.h>
#include <utils/pimpl.h>

#include "coreglobal.h"
#include "forwarddecl.h"

class NodePrivate;

class NodeUserData
{
public:
    virtual ~NodeUserData() = default;
};

class CORESHARED_EXPORT Node : public TreeNode<Node>
{
    PIMPL(Node)

public:
    Node();
    Node(NodePrivate*);
    ~Node() override;

    void setTransform(const Transform&);
    const Transform& transform() const;
    const Transform& globalTransform() const;

    const BoundingSphere& boundingSphere() const;

    std::shared_ptr<NodeUserData> userData() const;
    void setUserData(std::shared_ptr<NodeUserData>);

protected:
    void doAttach() override;
    void doDetach() override;

    std::unique_ptr<NodePrivate> m_; 
};

#endif // NODE_H
