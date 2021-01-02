#ifndef SCENEROOTNODE_H
#define SCENEROOTNODE_H

#include <utils/enumclass.h>

#include <core/node.h>

namespace trash
{
namespace core
{

class SceneRootNodePrivate;
class Scene;

class CORESHARED_EXPORT SceneRootNode : public Node
{
    PIMPL(SceneRootNode)

public:
    SceneRootNode(Scene*);
    Scene *scene() const;
};

} // namespace
} // namespace

#endif // SCENEROOTNODE_H
