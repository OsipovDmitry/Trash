#ifndef COREFORWARDDECL_H
#define COREFORWARDDECL_H

#include <array>
#include <vector>
#include <memory>
#include <inttypes.h>

namespace trash
{
namespace core
{

class AbstractController;
class Core;
class GraphicsController;
class AudioController;
class AbstractGame;
class NodeUserData;
class Node;
class ModelNode;
class TeapotNode;
class Camera;
class Scene;
class Light;

enum class LightType : uint32_t;

using LightsList = std::vector<std::shared_ptr<Light>>;
const int32_t MAX_LIGHTS_PER_NODE = 8;
using LightIndicesList = std::array<int32_t, MAX_LIGHTS_PER_NODE>;

} // namespace
} // namespace

#endif // COREFORWARDDECL_H
