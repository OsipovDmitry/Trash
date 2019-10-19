#ifndef NODE_H
#define NODE_H

#include <memory>

#include <utils/tree.h>
#include <utils/pimpl.h>

#include "coreglobal.h"
#include "forwarddecl.h"

class NodePrivate;

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

protected:
    void doAttach() override;
    void doDetach() override;

    std::unique_ptr<NodePrivate> m_;
};

#endif // NODE_H
