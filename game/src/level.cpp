#include <queue>
#include <set>

#include <utils/transform.h>
#include <utils/ray.h>

#include <core/autotransformnode.h>
#include <core/textnode.h>
#include <core/scene.h>
#include <core/scenerootnode.h>
#include <core/modelnode.h>
#include <core/primitivenode.h>
#include <core/nodevisitor.h>
#include <core/nodeintersectionvisitor.h>
#include <core/light.h>

#include "level.h"
#include "waypointsystem.h"

namespace trash
{
namespace game
{

Level::Level()
    : Scene()
    , m_wayPointSystem(std::make_unique<WayPointSystem>())
{
    auto rootNode = graphicsScene()->rootNode();

    m_wayPointNode = std::make_shared<core::PrimitiveNode>();
    //rootNode->attach(m_wayPointNode);

    m_wallsNode = std::make_shared<core::ModelNode>("office_walls.dae");
    rootNode->attach(m_wallsNode);

    m_floorNode = std::make_shared<core::ModelNode>("office_floor.dae");
    rootNode->attach(m_floorNode);

    auto bloomNode = std::make_shared<core::PrimitiveNode>();
    bloomNode->addBox(glm::vec4(glm::vec3(0.15f, 0.15f, 1.f) * 5.f, 1.0f),
                      utils::BoundingBox(glm::vec3(-4.1f-0.1f, 0.0f, -3.6f-0.1f), glm::vec3(-4.1f+0.1f, 3.0f, -3.6f+0.1f)),
                      false);
    rootNode->attach(bloomNode);

    auto light = std::make_shared<core::Light>(core::LightType::Point);
    light->setPosition(glm::vec3(-4.1f, 4.0f, -3.0f));
    light->setDirection(glm::vec3(0.0f, -1.0f, 0.0f));
    light->setSpotAngles(glm::vec2(2.4f));
    light->setColor(glm::vec3(0.15f, 0.15f, 1.f) * 15.f);
    light->setRadiuses(glm::vec2(6.0f, 4.0f));
    light->enableShadowOutside(true);
    graphicsScene()->attachLight(light);

    core::NodeSimpleVisitor nv([](std::shared_ptr<core::Node> node) {
        if (auto drawableNode = std::dynamic_pointer_cast<core::DrawableNode>(node))
            drawableNode->setIntersectionMode(core::IntersectionMode::UseGeometry);
    });

    m_wallsNode->accept(nv);
    m_floorNode->accept(nv);
}

Level::~Level()
{
}

WayPointSystem &Level::wayPointSystem()
{
    return *m_wayPointSystem;
}

const WayPointSystem &Level::wayPointSystem() const
{
    return *m_wayPointSystem;
}

ClosestWayPoints Level::findClosestWayPoints(const glm::vec3& pos, const std::vector<glm::vec2>& rayShifts) const
{
    std::multimap<float, std::shared_ptr<WayPoint>> result;

    auto t = m_wayPointSystem->wayPoints();

    for (auto wayPoint : t)
    {
        glm::vec3 rayDir = wayPoint->position - pos;
        auto distance = glm::length(rayDir);

        if (distance > 0.f)
        {
            rayDir /= distance;
            static const glm::vec3 rayUp = glm::vec3(0.f, 1.f, 0.f);
            const glm::vec3 rayRight = glm::cross(rayUp, -rayDir);

            bool isOk = true;

            for (const auto& shift : rayShifts)
            {
                const glm::vec3 rayPos = pos + shift.x * rayRight + shift.y * rayUp;

                core::NodeRayIntersectionVisitor visitor(utils::Ray(rayPos, rayDir));
                m_wallsNode->accept(visitor);
                const auto& intersections = visitor.intersectionData();

                if (!intersections.nodes.empty() && intersections.nodes.begin()->first < distance)
                {
                    isOk = false;
                    break;
                }
            }

            if (isOk)
            {
                result.insert({distance, wayPoint});

                for (const auto& shift : rayShifts)
                {
                    const glm::vec3 rayPos = pos + shift.x * rayRight + shift.y * rayUp;
                    //m_wayPointNode->addLine(glm::vec4(0.f, 1.f, 0.f, 1.f), {rayPos, rayPos + rayDir * distance}, {}, false);
                }
            }
        }
        else
        {
            result.insert({0.f, wayPoint});
        }
    }

    return result;
}

WayPointPath Level::buildRoute(const glm::vec3& startPoint, const glm::vec3& endPoint, const std::vector<glm::vec2>& rayShifts) const
{
    auto startWayPoint = std::make_shared<WayPoint>(startPoint);
    auto endWayPoint = std::make_shared<WayPoint>(endPoint);

    for (auto wayPoint : {startWayPoint, endWayPoint})
    {
        for (auto closestWayPoint : findClosestWayPoints(wayPoint->position, rayShifts))
            m_wayPointSystem->insertTwoSided(wayPoint, closestWayPoint.second);
    }

    auto route = m_wayPointSystem->findPath(startWayPoint, endWayPoint);

    //const_cast<Level*>(this)->updateGraphics();
    m_wayPointSystem->remove(startWayPoint);
    m_wayPointSystem->remove(endWayPoint);

    if (!route.empty())
    {
        route.erase(route.begin());
        route.erase(--(route.end()));
    }

    return route;
}

void Level::updateGraphics()
{
    m_wayPointNode->removeAllDrawables();
    m_wayPointNode->clear();

    std::set<std::shared_ptr<WayPoint>> wayPoints;
    for (auto it = m_wayPointSystem->m_graph.begin(); it != m_wayPointSystem->m_graph.end(); ++it)
    {
        for (auto wp : {it->first, it->second})
        {
            if (wayPoints.find(wp) == wayPoints.end())
            {
                wayPoints.insert(wp);
                m_wayPointNode->addLine(glm::vec4(1.f, 0.f, 0.f, 1.f),
                                        {wp->position, wp->position + glm::vec3(0.f, 3.f, 0.f)},
                                        {},
                                        false);

                auto autoTransform = std::make_shared<core::AutoTransformNode>();
                autoTransform->setTransform(utils::Transform::fromTranslation(wp->position + glm::vec3(0.f, 3.f, 0.f)));
                m_wayPointNode->attach(autoTransform);

                auto textNode = std::make_shared<core::TextNode>(wp->name, core::TextNodeAlignment::Center, core::TextNodeAlignment::Negative);
                textNode->setColor(glm::vec4(1.f, 0.f, 0.f, 1.f));
                autoTransform->attach(textNode);
            }

            const glm::vec3 d(0.f, it->first < it->second ? .5f : 1.f, 0.f);
            m_wayPointNode->addLine(glm::vec4(1.f, 1.f, 1.f, 1.f),
                                    {it->first->position + d, it->second->position + d},
                                    {glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f)},
                                    false);
        }
    }
}

} // namespace
} // namespace
