#include <glm/gtc/matrix_transform.hpp>

#include <core/core.h>
#include <core/camera.h>
#include <core/graphicscontroller.h>
#include <core/modelnode.h>
#include <core/scene.h>
#include <core/light.h>

#include <utils/transform.h>
#include <utils/boundingsphere.h>

#include <game/game.h>

#include "gameprivate.h"
#include "scene.h"
#include "person.h"
#include "floor.h"

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

    for (size_t i = 0; i < GamePrivate::numPersons; ++i)
    {
        m_->persons[i] = std::make_shared<Person>(GamePrivate::personsNames[i]);
        m_->scene->attachObject(m_->persons[i]);

        float angle = 2.0f * glm::pi<float>() * i / GamePrivate::numPersons;
        m_->persons[i]->graphicsNode()->setTransform(utils::Transform(
                                                         glm::vec3(1.f,1.f,1.f),
                                                         glm::quat_cast(glm::mat3x3(glm::vec3(-.7f,0.f,-.7f), glm::vec3(0.f,1.f,0.f), glm::vec3(.7f,0.f,-.7f))),
                                                         900.0f * glm::vec3(glm::cos(angle), 0.0f,glm::sin(angle))));
    }

//    for (float z = -1250; z <= 1250; z += 500)
//        for (float x = -1250; x <= 1250; x += 500)
        {
            auto l = std::make_shared<core::Light>(core::LightType::Direction);
//            l->setAttenuation(glm::vec3(0.00001, 0.0, 1));
//            l->setPosition(glm::vec3(x,500,z));
            l->setDirection(glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)));

            m_->scene->camera()->scene()->attachLight(l);
        }

    m_->floor = std::make_shared<Floor>();
    m_->scene->attachObject(m_->floor);

    const float r = 350;
    m_->scene->camera()->setProjectionMatrixAsPerspective(glm::pi<float>() * 0.25f, 1000.0f, 5000.0f);
    m_->scene->camera()->setViewMatrix(glm::lookAt(glm::vec3(5 * r, 4 * r, -5 * r), glm::vec3(0.0f, 500.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
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

    for (auto l : m_->scene->camera()->scene()->lights())
    {
        l->setColor(glm::vec3(1,1,1));
    }

    if (!m_->acivePerson.expired())
    {
        auto& lights = m_->scene->camera()->scene()->lights();
        auto& activeLights = m_->acivePerson.lock()->graphicsNode()->getLights();
        for (auto idx : activeLights)
        {
            if (idx >= 0)
            {
                lights.at(idx)->setColor(glm::vec3(1,0,0));
            }
        }
    }
}

void Game::doMouseClick(int x, int y)
{
    auto pickData = m_->scene->camera()->pickNode(x, y);
    if (pickData.node)
    {
        auto object = m_->scene->findObject(pickData.node.get());
        if (object == m_->floor)
        {
            if (!m_->acivePerson.expired())
                m_->acivePerson.lock()->moveTo(pickData.localCoord);
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
