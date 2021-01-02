#ifndef COREFORWARDDECL_H
#define COREFORWARDDECL_H

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
class SceneRootNode;
class AutoTransformNode;
class DrawableNode;
class TextNode;
class ModelNode;
class TeapotNode;
class Camera;
class Scene;
class Light;

struct PickData;
struct IntersectionData;

enum class LightType : uint32_t;
enum class TextNodeAlignment : uint8_t;
enum class IntersectionMode : uint8_t;



} // namespace
} // namespace

#endif // COREFORWARDDECL_H
