#include <glm/gtc/matrix_transform.hpp>

#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/modelnode.h>

#include <utils/transform.h>

#include <game/game.h>

static const float dist = 700.0f;
static const int n = 10;

void Game::doInitialize()
{
    static const std::array<std::string, 5> filenames { "dance1.dae", "dance2.dae", "dance3.dae", "dance4.dae", "dance5.dae" };

    auto rootNode = Core::instance().graphicsController().rootNode();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
        {
            auto node = std::make_shared<ModelNode>(filenames[rand() % filenames.size()]);
            node->setTransform(Transform(glm::vec3(1.f, 1.f, 1.f), glm::quat(1.f, 0.f, 0.f, 0.f), glm::vec3((i - .5f*n) * dist, 0.0f, (j - .5f*n) * dist)));
            rootNode->attach(node);

            node->playAnimation("", rand() % 10000);
        }

    Core::instance().graphicsController().setProjectionMatrix(glm::pi<float>() * 0.25f, 500.0f, 400000.0f);
}

void Game::doUpdate(uint64_t time, uint64_t dt)
{
    const float angle = (time - m_resetTime) * 0.00003f + 0.8f;
    const float radius = dist * n * 1.8f;
    const float height = dist * 8;

    Core::instance().graphicsController().setViewMatrix(
    glm::lookAt(radius * glm::vec3(cos(angle), 0.0f, sin(angle)) + glm::vec3(0.0f, height, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
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
