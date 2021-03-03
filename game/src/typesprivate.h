#ifndef TYPESPRIVATE_H
#define TYPESPRIVATE_H

#include <string>
#include <list>
#include <memory>
#include <map>

#include <glm/vec3.hpp>

namespace trash
{
namespace game
{

struct WayPoint
{
    WayPoint(const glm::vec3& p, const std::string& n = "") : position(p), name(n)
    {}
    glm::vec3 position;
    std::string name;
};

using WayPointPath = std::list<std::shared_ptr<WayPoint>>;
using ClosestWayPoints = std::multimap<float, std::shared_ptr<WayPoint>>;

} // namespace
} // namespace

#endif // TYPESPRIVATE_H
