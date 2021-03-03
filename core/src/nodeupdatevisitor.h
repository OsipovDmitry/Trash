#ifndef NODEUPDATEVISITOR_H
#define NODEUPDATEVISITOR_H

#include <core/nodevisitor.h>
#include <core/node.h>

#include "nodeprivate.h"

namespace trash
{
namespace core
{

class NodeUpdateVisitor : public NodeVisitor
{
public:
    NodeUpdateVisitor(uint64_t time, uint64_t dt) : NodeVisitor(), m_time(time), m_dt(dt) {}

    bool visit(std::shared_ptr<Node> node) override { node->m().doUpdate(m_time, m_dt); return true; }

private:
    uint64_t m_time, m_dt;
};

} // namespace
} // namespace

#endif // NODEUPDATEVISITOR_H
