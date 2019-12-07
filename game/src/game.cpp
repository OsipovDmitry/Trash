#include <glm/gtc/matrix_transform.hpp>

#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/modelnode.h>

#include <utils/transform.h>
#include <utils/boundingsphere.h>

#include <game/game.h>

#include "gameprivate.h"
#include "scene.h"
#include "person.h"
#include "floor.h"

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
        m_->persons[i]->graphicsNode()->setTransform(Transform(
                                                         glm::vec3(1.f,1.f,1.f),
                                                         glm::quat_cast(glm::mat3x3(glm::vec3(-.7f,0.f,-.7f), glm::vec3(0.f,1.f,0.f), glm::vec3(.7f,0.f,-.7f))),
                                                         900.0f * glm::vec3(glm::cos(angle), 0.0f,glm::sin(angle))));
    }

    m_->floor = std::make_shared<Floor>();
    m_->scene->attachObject(m_->floor);

    const float r = 350;
    Core::instance().graphicsController().setProjectionMatrix(glm::pi<float>() * 0.25f, 1000.0f, 5000.0f);
    Core::instance().graphicsController().setViewMatrix(glm::lookAt(glm::vec3(5 * r, 3 * r, -5 * r), glm::vec3(0.0f, 500.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
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
}

void Game::doMouseClick(int x, int y)
{
    auto pickData = Core::instance().graphicsController().pickNode(x, y);
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
