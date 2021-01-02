#include <glm/gtc/matrix_transform.hpp>

#include <core/core.h>
#include <core/camera.h>
#include <core/graphicscontroller.h>
#include <core/modelnode.h>
#include <core/scene.h>
#include <core/light.h>

#include <utils/transform.h>
#include <utils/boundingbox.h>
#include <utils/ray.h>
#include <utils/frustum.h>

#include <game/game.h>

#include "gameprivate.h"
#include "scene.h"
#include "person.h"
#include "floor.h"
#include "teapot.h"
#include "waypoint.h"

auto rnd = [](float from = 0.0f, float to = 1.0f)
{
    return static_cast<float>(rand()) / (RAND_MAX-1) * (to-from) + from;
};

namespace trash
{
namespace game
{

Game::Game()
    : m_(std::make_unique<GamePrivate>())
{
}

Game::~Game()
{
}

void Game::doInitialize()
{
    m_->scene = std::make_shared<Scene>();

    for (auto i = -4; i <= 4; ++i)
    {
        auto teapot = std::make_shared<Teapot>(glm::vec3(1.0f, 0.5f, 0.5f), 0.2f, 0.6f);
        teapot->graphicsNode()->setTransform(utils::Transform::fromTranslation(glm::vec3(i+(i%2), 0.0f, i-(i%2)) * 520.0f) * utils::Transform::fromScale(100.f));
        m_->scene->attachObject(teapot);

    }

    const float wpcoef = 700.0f;
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1,0,2) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1,0,1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(2,0,1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(2,0,-1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1,0,-1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1,0,-2) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-1,0,-2) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-1,0,-1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-2,0,-1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-2,0,1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-1,0,1) * wpcoef));
    m_->waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-1,0,2) * wpcoef));
    for (size_t i = 0; i < m_->waypoints.size(); ++i)
        m_->waypoints[i]->setNextWayPoint(m_->waypoints[(i+1)%m_->waypoints.size()]);

    for (size_t i = 0; i < GamePrivate::numPersons; ++i)
    {
        m_->persons[i] = std::make_shared<Person>(GamePrivate::personsNames[i]);
        m_->scene->attachObject(m_->persons[i]);

        float angle = 2.0f * glm::pi<float>() * i / GamePrivate::numPersons;
        m_->persons[i]->graphicsNode()->setTransform(utils::Transform(
                                                         glm::vec3(1.f,1.f,1.f),
                                                         glm::quat_cast(glm::mat3x3(glm::vec3(-.7f,0.f,-.7f), glm::vec3(0.f,1.f,0.f), glm::vec3(.7f,0.f,-.7f))),
                                                         900.0f * glm::vec3(glm::cos(angle), 0.0f,glm::sin(angle))));

        m_->persons[i]->moveTo(m_->waypoints[static_cast<size_t>(static_cast<float>(m_->waypoints.size()) / GamePrivate::numPersons * i)]);
    }

    m_->floor = std::make_shared<Floor>();
    m_->scene->attachObject(m_->floor);

    static const std::vector<glm::vec3> lightColors {
        glm::vec3(1,0,0),
                glm::vec3(0,1,0),
                glm::vec3(0,0,1),
                glm::vec3(1,1,0),
                glm::vec3(1,0,1),
                glm::vec3(0,1,1),
    };

    for (int z = 0; z < 50; ++z)
    {
        auto l = std::make_shared<core::Light>(core::LightType::Point);
        l->setPosition(glm::vec3(rnd(-2500, 2500), 200.0f, rnd(-2500, 2500)));
        l->setDirection(glm::vec3(0.0,-1.0,0.0));
        l->setSpotAngles(glm::vec2(1.0f, 1.5f));
        l->setColor(5.0f * lightColors.at(rand() % lightColors.size()));
        l->setRadiuses(glm::vec2(250.0f, 250.0f));
        l->enableShadowMap(false);
        m_->scene->scene()->attachLight(l);
    }

    auto l = std::make_shared<core::Light>(core::LightType::Direction);
    auto d = glm::vec3(2500.0f, 1000.0f, -2500.0f);
    l->setPosition(d);
    l->setDirection(-d);
    l->setColor(glm::vec3(0.5f, 0.5f, 0.5f));
    l->setSpotAngles(glm::vec2(1.0f, 5000.0f));
    m_->scene->scene()->attachLight(l);

    const float r = 400;
    m_->scene->scene()->camera()->setProjectionMatrixAsPerspective(glm::pi<float>() * 0.4f);
    m_->scene->scene()->camera()->setViewMatrix(glm::lookAt(glm::vec3(.0f, 0.f, 0.f), glm::vec3(.0f, 0.f, -1.f), glm::vec3(0.0f, 1.0f, 0.0f)));

//    const float r = 6;
//    m_->scene->camera()->setProjectionMatrixAsPerspective(glm::pi<float>() * 0.25f, 1.0f, 5000.0f);
//    m_->scene->camera()->setViewMatrix(glm::lookAt(glm::vec3(-3 * r, 3 * r, 5 * r), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
}

void Game::doUnitialize()
{
    m_->floor = nullptr;
    for (size_t i = 0; i < GamePrivate::numPersons; ++i)
        m_->persons[i] = nullptr;
    m_->scene = nullptr;
}

void Game::doUpdate(uint64_t time, uint64_t dt)
{
    m_->scene->update(time, dt);

    const float r = 300;
    const float t = time * 0.00005f + 10.0f;
    m_->scene->scene()->camera()->setViewMatrix(glm::lookAt(glm::vec3(11 * r * cos(t), 3 * r, -11 * r * sin(t)), glm::vec3(0.0f, 500.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

    for (auto person: m_->persons)
    {
        auto personNode = person->graphicsNode();
        //utils::Ray ray = personNode->globalTransform() * utils::Ray(personNode->boundingBox().center(), glm::vec3(0.f, 0.f, 1.f));

        auto transform = personNode->globalTransform() *
                glm::translate(glm::mat4x4(1.0f), personNode->boundingBox().center()) *
                glm::rotate(glm::mat4x4(1.0f), glm::pi<float>(), glm::vec3(0.f, 1.f, 0.f));

        utils::Frustum frustum(glm::perspective(glm::pi<float>()*0.25f, 1.0f, 100.0f, 3000.0f) * glm::inverse(transform));
        auto intersectionData = m_->scene->scene()->intersectScene(/*ray*/frustum);

        person->setText(person->name());

        bool isFound = false;
        for (const auto& intersection: intersectionData.nodes)
        {
            if (intersection.first < 0.f)
                continue;

            auto intersectedObject = m_->scene->findObject(intersection.second.get());
            if (intersectedObject == person)
                continue;

            for (auto intersectedPerson : m_->persons)
            {
                if (intersectedPerson == intersectedObject)
                {
                    person->setText(person->name() + " is looking at " + intersectedPerson->name());
                    isFound = true;
                    break;
                }
            }

            if (isFound)
                break;
        }
    }
}

void Game::doMouseClick(uint32_t, int x, int y)
{
    auto pickData = m_->scene->scene()->pickScene(x, y);
    if (pickData.node)
    {
        auto object = m_->scene->findObject(pickData.node.get());
        if (object == m_->floor)
        {
            if (!m_->acivePerson.expired())
            {
                auto waypoint = std::make_shared<WayPoint>(pickData.localCoord);
                m_->acivePerson.lock()->moveTo(waypoint);
            }
        }
        else
        {
            for (size_t i = 0; i < GamePrivate::numPersons; ++i)
            {
                if (m_->persons[i] == object)
                {
                    m_->acivePerson = m_->persons[i];
                    m_->acivePerson.lock()->wave();
                    break;
                }
            }
        }
    }
}

} // namespace
} // namespace
