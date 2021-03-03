#ifndef NODEINTERSECTIONVISITOR_H
#define NODEINTERSECTIONVISITOR_H

#include <map>

#include <utils/forwarddecl.h>
#include <utils/pimpl.h>

#include <core/coreglobal.h>
#include <core/nodevisitor.h>

namespace trash
{
namespace core
{

struct IntersectionData
{
    std::multimap<float, std::shared_ptr<DrawableNode>> nodes;
};

class NodeRayIntersectionVisitorPrivate;

class CORESHARED_EXPORT NodeRayIntersectionVisitor : public NodeVisitor
{
public:
    NodeRayIntersectionVisitor(const utils::Ray&);
    ~NodeRayIntersectionVisitor() override;

    bool visit(std::shared_ptr<Node>) override;

    const IntersectionData& intersectionData() const;

private:
    std::unique_ptr<NodeRayIntersectionVisitorPrivate> m_;
};

class NodeFrustumIntersectionVisitorPrivate;

class CORESHARED_EXPORT NodeFrustumIntersectionVisitor : public NodeVisitor
{
public:
    NodeFrustumIntersectionVisitor(const utils::Frustum&);
    ~NodeFrustumIntersectionVisitor() override;

    bool visit(std::shared_ptr<Node>) override;

    const IntersectionData& intersectionData() const;

private:
    std::unique_ptr<NodeFrustumIntersectionVisitorPrivate> m_;
};

} // namespace
} // namespace

#endif // NODEINTERSECTIONVISITOR_H
