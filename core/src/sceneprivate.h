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

struct PickData;
struct Buffer;
struct Texture;
struct Framebuffer;

class CameraPrivate;

class ScenePrivate
{
public:
    ScenePrivate(Scene*);

    void dirtyLightParams(Light*);
    std::shared_ptr<Buffer> getLightParamsBuffer();
    void updateLightParams();

    void dirtyShadowMap(Light*);
    std::shared_ptr<Texture> getLightsShadowMaps();
    void updateShadowMaps();

    void updateShadowMap(std::shared_ptr<Light>);

    void renderScene(uint64_t, uint64_t, const CameraPrivate&);
    PickData pickScene(int32_t, int32_t, const trash::core::CameraPrivate &);

    std::shared_ptr<SceneRootNode> rootNode;
    std::shared_ptr<LightsList> lights;
    std::shared_ptr<Buffer> lightsUbo;
    std::shared_ptr<Texture> lightsShadowMaps;

    static const float ShadowMapZNear;
    static const float ShadowMapZFar;
    static const int32_t ShadowMapSize;

    std::set<uint32_t> freeLights;
    std::set<uint32_t> dirtyLights, dirtyShadowMaps;
    bool allLightsAreDirty, allShadowMapsAreDirty;
};

} // namespace
} // namespace

#endif // SCENEPRIVATE_H
