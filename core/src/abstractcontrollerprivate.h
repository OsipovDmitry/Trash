#ifndef ABSTRACTCONTROLLERPRIVATE_H
#define ABSTRACTCONTROLLERPRIVATE_H

#include <deque>

#include <core/abstractcontroller.h>

class AbstractControllerPrivate
{
public:
    virtual ~AbstractControllerPrivate() = default;

    std::deque<std::shared_ptr<AbstractController::Message>> messages;
};

#endif // ABSTRACTCONTROLLERPRIVATE_H
