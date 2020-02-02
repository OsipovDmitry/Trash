#ifndef SCENEPRIVATE_H
#define SCENEPRIVATE_H

#include <memory>
#include <vector>
#include <set>

#include <core/forwarddecl.h>
#include <core/node.h>

#include "nodeprivate.h"

namespace trash
{
namespace core
{

class SceneRootNode : public Node
{
public:
    SceneRootNode(Scene*);
    Scene *scene() const;
private:
    Scene *m_scene;
};

struct Buffer;

class ScenePrivate
{
public:
    void dirtyLight(Light*);
    std::shared_ptr<Buffer> getLightsBuffer();

    std::shared_ptr<SceneRootNode> rootNode;
    LightsList lights;
    std::shared_ptr<Buffer> lightsUbo;

    std::set<LightsList::difference_type> dirtyLights;
    bool dirtyAllLights;
};

} // namespace
} // namespace

#endif // SCENEPRIVATE_H
