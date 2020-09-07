#ifndef AUTOTRANSFORMNODEPRIVATE_H
#define AUTOTRANSFORMNODEPRIVATE_H

#include <core/forwarddecl.h>

#include "nodeprivate.h"

namespace trash
{
namespace core
{

class AutoTransformNodePrivate : public NodePrivate
{
public:
    AutoTransformNodePrivate(Node&);

    void doUpdate(uint64_t, uint64_t, bool) override;
};

} // namespace
} // namespace

#endif // AUTOTRANSFORMNODEPRIVATE_H
