#ifndef MODELNODEPRIVATE_H
#define MODELNODEPRIVATE_H

#include <string>
#include <vector>

#include "nodeprivate.h"

struct Model;
struct Buffer;

class ModelNodePrivate : public NodePrivate
{
public:
    ModelNodePrivate(Node&);

    void doUpdate(uint64_t, uint64_t) override;

    std::shared_ptr<Model> model;
    std::shared_ptr<Buffer> bonesBuffer;
    std::string animationName;
    uint32_t timeOffset;
    bool showBones;
};

#endif // MODELNODEPRIVATE_H
