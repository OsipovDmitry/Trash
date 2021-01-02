#ifndef SCENEPRIVATE_H
#define SCENEPRIVATE_H

#include <memory>
#include <vector>
#include <set>

#include <core/types.h>
#include <core/node.h>

#include "nodeprivate.h"
#include "typesprivate.h"

namespace trash
{
namespace core
{

struct Buffer;
struct Texture;
struct Framebuffer;

class Drawable;

class ScenePrivate
{
public:
    ScenePrivate(Scene*);

    void attachLight(std::shared_ptr<Light>);
    bool detachLight(std::shared_ptr<Light>);

    void dirtyLightParams(Light*);
    void dirtyShadowMap(Light*);

    static void dirtyNodeLightIndices(Node&);
    static void dirtyNodeShadowMaps(Node&);
    static utils::Transform calcLightViewTransform(std::shared_ptr<Light>);
    static glm::mat4x4 calcLightProjMatrix(std::shared_ptr<Light>, const std::pair<float, float>&);

    void renderScene(uint64_t, uint64_t);
    PickData pickScene(int32_t, int32_t);
    IntersectionData intersectScene(const utils::Ray&);
    IntersectionData intersectScene(const utils::Frustum&);

    Scene& thisScene;
    std::shared_ptr<SceneRootNode> rootNode;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<LightsList> lights;
    std::shared_ptr<Buffer> lightsUbo;
    std::shared_ptr<Framebuffer> lightsFramebuffer;
    std::shared_ptr<Texture> lightsShadowMaps;
    std::shared_ptr<Texture> iblDiffuseMap, iblSpecularMap, iblBrdfLutMap;
    float iblContribution;
    std::shared_ptr<Drawable> backgroundDrawable;
    std::shared_ptr<Drawable> postEffectDrawable;
    std::array<std::shared_ptr<Drawable>, numElementsLightType()> lightsDrawables;
    std::shared_ptr<Drawable> iblDrawable;

    float cameraMinZNear, cameraMaxZFar;
    float shadowMapMinZNear, shadowMapMaxZFar;
    int32_t shadowMapSize;

    std::set<uint32_t> freeLightIndices;
    std::set<uint32_t> dirtyLights, dirtyShadowMaps;

    bool useDeferredTechnique;
};

} // namespace
} // namespace

#endif // SCENEPRIVATE_H
