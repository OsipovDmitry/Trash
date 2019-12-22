#ifndef ABSTRACTCONTROLLERPRIVATE_H
#define ABSTRACTCONTROLLERPRIVATE_H

#include <deque>

#include <core/abstractcontroller.h>

namespace trash
{
namespace core
{

class AbstractControllerPrivate
{
public:
    virtual ~AbstractControllerPrivate() = default;

    std::deque<std::shared_ptr<AbstractController::Message>> messages;
};

} // namespace
} // namespace

#endif // ABSTRACTCONTROLLERPRIVATE_H
