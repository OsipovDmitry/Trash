#include <core/scenerootnode.h>

#include "scenerootnodeprivate.h"

namespace trash
{
namespace core
{

SceneRootNode::SceneRootNode(Scene* scene)
    : Node(new SceneRootNodePrivate(*this, scene))
{
}

Scene *SceneRootNode::scene() const
{
    return m().scene;
}

} // namespace
} // namespace
