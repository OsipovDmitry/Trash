#include <glm/gtc/matrix_transform.hpp>

#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/modelnode.h>

#include <utils/transform.h>

#include <game/game.h>

void Game::doInitialize()
{
    const float dist = 250.0f;

    auto node1 = std::make_shared<ModelNode>("dance1.dae");
    node1->setTransform(Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)));

    auto node2 = std::make_shared<ModelNode>("dance2.dae");
    node2->setTransform(Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(-dist, 0.0f, -dist)));

    auto node3 = std::make_shared<ModelNode>("dance3.dae");
    node3->setTransform(Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(-dist, 0.0f, +dist)));

    auto node4 = std::make_shared<ModelNode>("dance4.dae");
    node4->setTransform(Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(+dist, 0.0f, +dist)));

    auto node5 = std::make_shared<ModelNode>("dance5.dae");
    node5->setTransform(Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(+dist, 0.0f, -dist)));

    auto rootNode = Core::instance().graphicsController().rootNode();
    rootNode->attach(node1);
    rootNode->attach(node2);
    rootNode->attach(node3);
    rootNode->attach(node4);
    rootNode->attach(node5);
}

void Game::doUpdate(uint64_t time, uint64_t dt)
{
    const float angle = (time - m_resetTime) * 0.00006f + 0.8f;
    const float radius = 1000.0f;
    const float height = 500.0f;

    Core::instance().graphicsController().setViewMatrix(
    glm::lookAt(radius * glm::vec3(cos(angle), 0.0f, sin(angle)) + glm::vec3(0.0f, height, 0.0f),
                glm::vec3(0.0f, height*0.33f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f))
    );

    m_currTime = time;
}

void Game::doMouseClick(int, int)
{
    m_resetTime = m_currTime;
}

Game::Game()
    : AbstractGame()
{ 
}
