#ifndef FLOOR_H
#define FLOOR_H

#include "object.h"

class ModelNode;

class Floor : public Object
{
public:
    Floor();

protected:
    std::shared_ptr<ModelNode> m_modelNode;

};

#endif // FLOOR_H
