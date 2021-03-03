#include <glm/geometric.hpp>

#include "waypointsystem.h"

namespace trash
{
namespace game
{

WayPointSystem::WayPointSystem()
{
}

WayPointSystem::~WayPointSystem()
{   
}

std::set<std::shared_ptr<WayPoint>> WayPointSystem::wayPoints() const
{
    std::set<std::shared_ptr<WayPoint>> result;
    for (auto pair : m_graph)
    {
        result.insert(pair.first);
        result.insert(pair.second);
    }
    return result;
}

void WayPointSystem::insert(std::shared_ptr<trash::game::WayPoint> p1, std::shared_ptr<trash::game::WayPoint> p2)
{
    if (!p1 || !p2 /*|| p1 == p2*/)
        return;

    auto iters = m_graph.equal_range(p1);
    if (iters.first != m_graph.end())
        for (auto it = iters.first; it != iters.second; ++it)
            if (it->second == p2)
                return;

    m_graph.insert({p1, p2});
}

void WayPointSystem::insertTwoSided(std::shared_ptr<WayPoint> p1, std::shared_ptr<WayPoint> p2)
{
    insert(p1, p2);
    insert(p2, p1);
}

void WayPointSystem::remove(std::shared_ptr<WayPoint> p1, std::shared_ptr<WayPoint> p2)
{
    if (!p1 || !p2)
        return;

    auto iters = m_graph.equal_range(p1);
    if (iters.first != m_graph.end())
        for (auto it = iters.first; it != iters.second; ++it)
            if (it->second == p2)
            {
                m_graph.erase(it);
                break;
            }
}

void WayPointSystem::removeTwoSided(std::shared_ptr<WayPoint> p1, std::shared_ptr<WayPoint> p2)
{
    remove(p1, p2);
    remove(p2, p1);
}

void WayPointSystem::remove(std::shared_ptr<WayPoint> p)
{
    if (!p)
        return;

    for (auto it = m_graph.begin(); it != m_graph.end(); )
    {
        if (it->first == p || it->second == p)
            it = m_graph.erase(it);
        else
            ++it;
    }
}

WayPointPath WayPointSystem::findPath(std::shared_ptr<WayPoint> start, std::shared_ptr<WayPoint> end) const
{
    std::multimap<float, std::shared_ptr<WayPoint>> frontier;
    frontier.insert({0.0f, start});

    std::map<std::shared_ptr<WayPoint>, std::shared_ptr<WayPoint>> cameFrom;
    cameFrom.insert({start, nullptr});

    std::map<std::shared_ptr<WayPoint>, float> costSoFar;
    costSoFar.insert({start, 0.0f});

    while (!frontier.empty())
    {
        auto current = frontier.begin()->second;
        frontier.erase(frontier.begin());

        if (current == end)
            break;

        auto iters = m_graph.equal_range(current);
        if (iters.first != m_graph.end())
            for (auto next = iters.first; next != iters.second; ++next)
            {
                float newCost = costSoFar[current] + glm::distance(current->position, next->second->position);
                auto it = costSoFar.find(next->second);
                if (it == costSoFar.end())
                    costSoFar.insert({next->second, std::numeric_limits<float>::max()});
                if (newCost < costSoFar[next->second])
                {
                    costSoFar[next->second] = newCost;
                    float priority = newCost + glm::distance(current->position, next->second->position);
                    frontier.insert({priority, next->second});
                    cameFrom[next->second] = current;
                }
            }
    }

    if (cameFrom.find(end) == cameFrom.end())
        return WayPointPath();

    WayPointPath path{end};
    std::shared_ptr<WayPoint> wp = end;
    while (wp != start)
    {
        wp = cameFrom[wp];
        path.push_front(wp);
    }

    return path;
}


} // namespace
} // namespace
