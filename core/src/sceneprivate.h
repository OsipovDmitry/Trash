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

class Drawable;
class CameraPrivate;

class ScenePrivate
{
public:
    ScenePrivate(Scene*);

    void attachLight(std::shared_ptr<Light>);
    bool detachLight(std::shared_ptr<Light>);

    void dirtyLightParams(Light*);
    std::shared_ptr<Buffer> getLightParamsBuffer();
    void updateLightParams();

    void dirtyShadowMap(Light*);
    std::shared_ptr<Texture> getLightsShadowMaps();
    void updateShadowMaps();
    void updateShadowMap(std::shared_ptr<Light>);

    static void dirtyNodeLightIndices(Node&);
    static void dirtyNodeShadowMaps(Node&);

    void renderScene(uint64_t, uint64_t);
    PickData pickScene(int32_t, int32_t);

    std::pair<float, float> calculateZPlanes(const glm::mat4x4&, float) const;

    Scene& thisScene;
    std::shared_ptr<SceneRootNode> rootNode;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<LightsList> lights;
    std::shared_ptr<Buffer> lightsUbo;
    std::shared_ptr<Texture> lightsShadowMaps;
    std::shared_ptr<Drawable> backgroundDrawable;

    static const float CameraMinZNear;
    static const float ShadowMapMinZNear;
    static const int32_t ShadowMapSize;

    std::set<uint32_t> freeLightIndices;
    std::set<uint32_t> dirtyLights, dirtyShadowMaps;
};

} // namespace
} // namespace

#endif // SCENEPRIVATE_H
