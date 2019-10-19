#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/trianglenode.h>

#include <utils/transform.h>

#include <game/game.h>

void Game::doInitialize()
{
    auto rootNode = Core::instance().graphicsController().rootNode();

    for (int i = 0; i < 3; ++i)
    {
        auto bigTri = std::make_shared<TriangleNode>();
        rootNode->attach(bigTri);

        for (int j = 0; j < 3; ++j)
        {
            auto smallTri = std::make_shared<TriangleNode>();
            bigTri->attach(smallTri);
        }
    }
}

void Game::doUpdate(uint64_t time, uint64_t)
{
    float t = time * 0.001f * 0.5f;
    auto rootNode = Core::instance().graphicsController().rootNode();

    for (int i = 0; i < 3; ++i)
    {
        auto bigTri = rootNode->children().at(i);
        bigTri->setTransform(Transform(glm::vec3(1.0f, 1.0f, 1.0f),
                                       glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                                       glm::vec3(0.6f * std::cosf(t + 6.28f * i / 3), 0.6f * std::sinf(t + 6.28f * i / 3), 0.0f)));

        for (int j = 0; j < 3; ++j)
        {
            auto smallTri = bigTri->children().at(j);
            smallTri->setTransform(Transform(glm::vec3(0.3f, 0.3f, 1.0f),
                                             glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                                             glm::vec3(0.4f * std::cosf(-t * 0.7f + 6.28f * j / 3), 0.4f * std::sinf(-t * 0.7f + 6.28f * j / 3), 0.0f)));
        }
    }
}

Game::Game()
    : AbstractGame()
{ 
}
