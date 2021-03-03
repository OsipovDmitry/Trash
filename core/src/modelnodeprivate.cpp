#include "modelnodeprivate.h"

namespace trash
{
namespace core
{

ModelNodePrivate::ModelNodePrivate(Node& node)
    : NodePrivate(node)
    , animationName("")
    , animationTime(0)
    , showBones(false)
{
}

} // namespace
} // namespace
