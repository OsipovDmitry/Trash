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
#include "teapot.h"

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

//    const int N = 7;
//    for (int x = 0; x < N; ++x)
//        for (int y = 0; y < N; ++y)
//        {
//            auto teapot = std::make_shared<Teapot>(glm::vec3(1.0f, 1.0f, 1.0f), static_cast<float>(y) / (N-1), static_cast<float>(x) / (N-1));
//            teapot->graphicsNode()->setTransform(utils::Transform(glm::vec3(1.f, 1.f, 1.f),
//                                                                  glm::quat(1.f, 0.f, 0.f, 0.f),
//                                                                  glm::vec3((x - (N-1)*.5f) * 4, (y - (N-1)*.5f) * 4, 0.f)));
//            m_->scene->attachObject(teapot);
//        }



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

    m_->floor = std::make_shared<Floor>();
    m_->scene->attachObject(m_->floor);


    std::vector<glm::vec3> qwe {glm::vec3(-800,1000,-800), /*glm::vec3(800,1000,800), glm::vec3(-800,1000,800), glm::vec3(800,1000,-800)*/ };
    for (const auto& p: qwe)
        {
            auto l = std::make_shared<core::Light>(/*core::LightType::Direction*/);
            l->setAttenuation(glm::vec3(0.0, 0.001, 1));
            l->setPosition(p);
            l->setDirection(-p/*glm::vec3(0.0,-1.0,0.0)*/);
            l->setSpotAngles(glm::vec2(0.2f, 1.55f));

            m_->scene->camera()->scene()->attachLight(l);
        }



    const float r = 350;
    m_->scene->camera()->setProjectionMatrixAsPerspective(glm::pi<float>() * 0.25f, 1000.0f, 5000.0f);
    m_->scene->camera()->setViewMatrix(glm::lookAt(glm::vec3(5 * r, 4 * r, -5 * r), glm::vec3(0.0f, 500.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

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

//    const float r = 10;
//    m_->scene->camera()->setViewMatrix(glm::lookAt(glm::vec3(2 * r * cos(time * 0.0005), 1 * r, -2 * r * sin(time * 0.0005)),
//                                                   glm::vec3(0.0f, 0.0f, 0.0f),
//                                                   glm::vec3(0.0f, 1.0f, 0.0f)));

//    for (auto l : m_->scene->camera()->scene()->lights())
//    {
//        l->setColor(glm::vec3(1,1,1));
//    }

//    if (!m_->acivePerson.expired())
//    {
//        auto& lights = m_->scene->camera()->scene()->lights();
//        auto& activeLights = m_->acivePerson.lock()->graphicsNode()->getLights();
//        for (auto idx : activeLights)
//        {
//            if (idx >= 0)
//            {
//                lights.at(idx)->setColor(glm::vec3(1,0,0));
//            }
//        }
//    }
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
