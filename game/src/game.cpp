#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/modelnode.h>

#include <utils/transform.h>

#include <game/game.h>

void Game::doInitialize()
{
    auto node1 = std::make_shared<ModelNode>("Samba_Dancing.dae");
    node1->setTransform(Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -200.0f, -550.0f)));

    auto rootNode = Core::instance().graphicsController().rootNode();
    rootNode->attach(node1);
}

void Game::doUpdate(uint64_t time, uint64_t dt)
{
}

Game::Game()
    : AbstractGame()
{ 
}
