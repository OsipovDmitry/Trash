#ifndef WAYPOINTSYSTEM_H
#define WAYPOINTSYSTEM_H

#include <memory>
#include <map>
#include <set>

#include <glm/vec3.hpp>

#include "typesprivate.h"

namespace trash
{
namespace game
{

class WayPointSystem
{
public:
    WayPointSystem();
    virtual ~WayPointSystem();

    std::set<std::shared_ptr<WayPoint> > wayPoints() const;

    void insert(std::shared_ptr<WayPoint>, std::shared_ptr<WayPoint>);
    void insertTwoSided(std::shared_ptr<WayPoint>, std::shared_ptr<WayPoint>);
    void remove(std::shared_ptr<WayPoint>, std::shared_ptr<WayPoint>);
    void removeTwoSided(std::shared_ptr<WayPoint>, std::shared_ptr<WayPoint>);
    void remove(std::shared_ptr<WayPoint>);

    WayPointPath findPath(std::shared_ptr<WayPoint>, std::shared_ptr<WayPoint>) const;

private:
    std::multimap<std::shared_ptr<WayPoint>, std::shared_ptr<WayPoint>> m_graph;

    friend class Level;
};

} // namespace
} // namespace

#endif // WAYPOINTSYSTEM_H
