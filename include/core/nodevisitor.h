#ifndef NODEVISITOR_H
#define NODEVISITOR_H

#include <functional>
#include <memory>

#include <utils/noncopyble.h>

#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class NodeVisitor
{
    NONCOPYBLE(NodeVisitor)

public:
    NodeVisitor() {}
    virtual ~NodeVisitor() = default;
    virtual bool visit(std::shared_ptr<Node>) { return true; }
};

class NodeSimpleVisitor : public NodeVisitor
{
public:
    template <typename T>
    NodeSimpleVisitor(T func) : m_func(func) {}

    bool visit(std::shared_ptr<Node> node) override { m_func(node); return true; }

private:
    std::function<void(std::shared_ptr<Node>)> m_func;
};

} // namespace
} // namespace

#endif // NODEVISITOR_H
