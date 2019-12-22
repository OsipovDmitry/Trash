#ifndef SCENEPRIVATE_H
#define SCENEPRIVATE_H

#include <memory>
#include <vector>
#include <set>

#include <core/forwarddecl.h>

#include "renderer.h"

namespace trash
{
namespace core
{

class ScenePrivate
{
public:
    void dirtyLight(Light*);
    std::shared_ptr<Buffer> getLightsBuffer();


    std::shared_ptr<Node> rootNode;
    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Buffer> lightsUbo;

    std::set<Light*> dirtyLights;
    bool dirtyAllLights;
};

} // namespace
} // namespace

#endif // SCENEPRIVATE_H
