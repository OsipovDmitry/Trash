#ifndef MODELNODEPRIVATE_H
#define MODELNODEPRIVATE_H

#include <string>

#include "nodeprivate.h"

namespace trash
{
namespace core
{

struct Model;
struct Buffer;

class ModelNodePrivate : public NodePrivate
{
public:
    ModelNodePrivate(Node&);

    std::shared_ptr<Model> model;
    std::shared_ptr<Buffer> bonesBuffer;
    std::string animationName;
    uint64_t animationTime;
    bool showBones;
};

} // namespace
} // namespace

#endif // MODELNODEPRIVATE_H
