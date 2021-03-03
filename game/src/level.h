#ifndef LEVEL_H
#define LEVEL_H

#include <map>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <core/forwarddecl.h>

#include "scene.h"
#include "typesprivate.h"

namespace trash
{
namespace game
{

class WayPointSystem;

class Level : public Scene
{
public:
    Level();
    ~Level() override;

    WayPointSystem& wayPointSystem();
    const WayPointSystem& wayPointSystem() const;

    ClosestWayPoints findClosestWayPoints(const glm::vec3&, const std::vector<glm::vec2>&) const;
    WayPointPath buildRoute(const glm::vec3&, const glm::vec3&, const std::vector<glm::vec2>&) const;

    std::shared_ptr<const core::Node> floorNode() const { return m_floorNode; }

    void updateGraphics();

protected:
    std::shared_ptr<core::ModelNode> m_floorNode, m_wallsNode;
    std::shared_ptr<core::PrimitiveNode> m_wayPointNode;
    std::unique_ptr<WayPointSystem> m_wayPointSystem;

};

} // namespace
} // namespace

#endif // LEVEL_H
