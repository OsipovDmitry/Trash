#include <glm/gtc/matrix_transform.hpp>

#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/modelnode.h>
#include <core/drawablenode.h>
#include <core/primitivenode.h>
#include <core/scenerootnode.h>
#include <core/scene.h>
#include <core/light.h>
#include <core/nodeintersectionvisitor.h>

#include <utils/transform.h>
#include <utils/boundingbox.h>
#include <utils/ray.h>
#include <utils/frustum.h>

#include <game/game.h>

#include "gameprivate.h"
#include "person.h"
#include "level.h"
#include "floor.h"
#include "teapot.h"
#include "waypointsystem.h"

auto rnd = [](float from = 0.0f, float to = 1.0f)
{
    return static_cast<float>(rand()) / (RAND_MAX) * (to-from) + from;
};


namespace trash
{
namespace game
{

std::vector<std::shared_ptr<trash::core::Light>> lights;
std::vector<glm::vec3> lightsPos;
std::vector<float> lightsCW, lightsR;
std::vector<std::shared_ptr<WayPoint>> waypoints;

Game::Game()
    : m_(std::make_unique<GamePrivate>())
{
    utils::Transform t(glm::vec3(1.f, 2.f, 3.f),
                       glm::quat_cast(glm::rotate(glm::mat4(1.f), 30.f, glm::normalize(glm::vec3(1.f, 2.f, 3.f)))),
                       glm::vec3(10.f, 5.f, 7.f));

    auto a = t * utils::Ray(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 4.f, 2.f));

}

Game::~Game()
{
}

void Game::doInitialize()
{
    m_->scene = std::make_shared<Level>();

//    for (auto i = -4; i <= 4; ++i)
//    {
//        auto teapot = std::make_shared<Teapot>(glm::vec3(1.0f, 0.5f, 0.5f), 0.2f, 0.6f);
//        teapot->graphicsNode()->setTransform(utils::Transform::fromTranslation(glm::vec3(i+(i%2), 0.0f, i-(i%2)) * 2.6f) * utils::Transform::fromScale(.5f));
//        m_->scene->attachObject(teapot);
//    }

    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(9.60f, 0.0f, -17.40f), "0"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(11.83f, 0.0f, -17.40f), "1"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(13.84f, 0.0f, -16.58f), "2"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(7.28f, 0.0f, -17.54f), "3"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(4.07f, 0.0f, -12.58f), "4"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(7.28f, 0.0f, -8.18f), "5"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(9.60f, 0.0f, -8.18f), "6"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(11.76f, 0.0f, -8.18f), "7"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(13.72f, 0.0f, -10.06f), "8"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(0.50f, 0.0f, -8.18f), "9"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-0.65f, 0.0f, -8.18f), "10"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-4.00f, 0.0f, -8.95f), "11"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(9.60f, 0.0f, -5.19f), "12"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(14.66f, 0.0f, -5.19f), "13"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(14.66f, 0.0f, -2.87f), "14"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(12.55f, 0.0f, 0.00f), "15"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(9.46f, 0.0f, 2.09f), "16"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(8.46f, 0.0f, 2.09f), "17"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(5.86f, 0.0f, 0.00f), "18"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(4.64f, 0.0f, -2.88f), "19"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(4.64f, 0.0f, -5.19f), "20"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1.42f, 0.0f, -5.19f), "21"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1.42f, 0.0f, -1.21f), "22"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-0.77f, 0.0f, -1.21f), "23"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-3.98f, 0.0f, -3.02f), "24"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1.42f, 0.0f, 5.31f), "25"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(3.53f, 0.0f, 5.31f), "26"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(10.25f, 0.0f, 7.37f), "27"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(14.66f, 0.0f, 10.19f), "28"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(14.66f, 0.0f, 12.33f), "29"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(14.66f, 0.0f, 14.54f), "30"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(12.36f, 0.0f, 17.61f), "31"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(4.49f, 0.0f, 12.33f), "32"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(1.42f, 0.0f, 12.33f), "33"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(4.49f, 0.0f, 14.78f), "34"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(5.96f, 0.0f, 17.58f), "35"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(3.33f, 0.0f, 19.36f), "36"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(2.33f, 0.0f, 19.36f), "37"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-3.41f, 0.0f, 19.36f), "38"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-4.41f, 0.0f, 19.36f), "39"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-0.47f, 0.0f, 17.58f), "40"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-7.59f, 0.0f, 17.58f), "41"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-5.50f, 0.0f, 14.44f), "42"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-5.50f, 0.0f, 12.33f), "43"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-9.18f, 0.0f, 12.33f), "44"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-11.45f, 0.0f, 12.33f), "45"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-14.54f, 0.0f, 16.56f), "46"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-16.26f, 0.0f, 11.28f), "47"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-16.26f, 0.0f, 10.28f), "48"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-14.59f, 0.0f, 6.95f), "49"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-9.18f, 0.0f, 9.22f), "50"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-7.04f, 0.0f, 9.22f), "51"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-7.04f, 0.0f, 2.07f), "52"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-4.04f, 0.0f, 5.55f), "53"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-9.18f, 0.0f, 2.07f), "54"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-9.18f, 0.0f, -2.00f), "55"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-11.37f, 0.0f, -2.00f), "56"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-14.69f, 0.0f, -0.38f), "57"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-9.18f, 0.0f, -5.22f), "58"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-11.30f, 0.0f, -5.22f), "59"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-14.53f, 0.0f, -6.66f), "60"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-16.29f, 0.0f, -9.71f), "61"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-16.29f, 0.0f, -10.71f), "62"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-14.5f, 0.0f, -14.16f), "63"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-11.28f, 0.0f, -17.31f), "64"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-9.18f, 0.0f, -17.31f), "65"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-7.00f, 0.0f, -17.31f), "66"));
    waypoints.push_back(std::make_shared<WayPoint>(glm::vec3(-3.69f, 0.0f, -15.24f), "67"));

    auto& wayPointSystem = m_->scene->wayPointSystem();
    wayPointSystem.insertTwoSided(waypoints[0], waypoints[6]);
    wayPointSystem.insertTwoSided(waypoints[6], waypoints[12]);
    wayPointSystem.insertTwoSided(waypoints[12], waypoints[13]);
    wayPointSystem.insertTwoSided(waypoints[12], waypoints[20]);
    wayPointSystem.insertTwoSided(waypoints[20], waypoints[21]);
    wayPointSystem.insertTwoSided(waypoints[21], waypoints[22]);
    wayPointSystem.insertTwoSided(waypoints[22], waypoints[25]);
    wayPointSystem.insertTwoSided(waypoints[25], waypoints[33]);
    wayPointSystem.insertTwoSided(waypoints[32], waypoints[33]);
    wayPointSystem.insertTwoSided(waypoints[29], waypoints[32]);
    wayPointSystem.insertTwoSided(waypoints[33], waypoints[43]);
    wayPointSystem.insertTwoSided(waypoints[43], waypoints[44]);
    wayPointSystem.insertTwoSided(waypoints[44], waypoints[50]);
    wayPointSystem.insertTwoSided(waypoints[50], waypoints[54]);
    wayPointSystem.insertTwoSided(waypoints[54], waypoints[55]);
    wayPointSystem.insertTwoSided(waypoints[55], waypoints[58]);
    wayPointSystem.insertTwoSided(waypoints[58], waypoints[65]);

    wayPointSystem.insertTwoSided(waypoints[0], waypoints[1]);
    wayPointSystem.insertTwoSided(waypoints[1], waypoints[2]);
    wayPointSystem.insertTwoSided(waypoints[0], waypoints[3]);
    wayPointSystem.insertTwoSided(waypoints[3], waypoints[4]);
    wayPointSystem.insertTwoSided(waypoints[4], waypoints[5]);
    wayPointSystem.insertTwoSided(waypoints[4], waypoints[9]);
    wayPointSystem.insertTwoSided(waypoints[9], waypoints[10]);
    wayPointSystem.insertTwoSided(waypoints[10], waypoints[11]);
    wayPointSystem.insertTwoSided(waypoints[5], waypoints[6]);
    wayPointSystem.insertTwoSided(waypoints[6], waypoints[7]);
    wayPointSystem.insertTwoSided(waypoints[7], waypoints[8]);
    wayPointSystem.insertTwoSided(waypoints[13], waypoints[14]);
    wayPointSystem.insertTwoSided(waypoints[14], waypoints[15]);
    wayPointSystem.insertTwoSided(waypoints[15], waypoints[16]);
    wayPointSystem.insertTwoSided(waypoints[16], waypoints[17]);
    wayPointSystem.insertTwoSided(waypoints[17], waypoints[18]);
    wayPointSystem.insertTwoSided(waypoints[18], waypoints[19]);
    wayPointSystem.insertTwoSided(waypoints[19], waypoints[20]);
    wayPointSystem.insertTwoSided(waypoints[22], waypoints[23]);
    wayPointSystem.insertTwoSided(waypoints[23], waypoints[24]);
    wayPointSystem.insertTwoSided(waypoints[25], waypoints[26]);
    wayPointSystem.insertTwoSided(waypoints[26], waypoints[27]);
    wayPointSystem.insertTwoSided(waypoints[27], waypoints[28]);
    wayPointSystem.insertTwoSided(waypoints[28], waypoints[29]);
    wayPointSystem.insertTwoSided(waypoints[29], waypoints[30]);
    wayPointSystem.insertTwoSided(waypoints[30], waypoints[31]);
    wayPointSystem.insertTwoSided(waypoints[32], waypoints[34]);
    wayPointSystem.insertTwoSided(waypoints[34], waypoints[35]);
    wayPointSystem.insertTwoSided(waypoints[35], waypoints[36]);
    wayPointSystem.insertTwoSided(waypoints[36], waypoints[37]);
    wayPointSystem.insertTwoSided(waypoints[37], waypoints[40]);
    wayPointSystem.insertTwoSided(waypoints[40], waypoints[38]);
    wayPointSystem.insertTwoSided(waypoints[38], waypoints[39]);
    wayPointSystem.insertTwoSided(waypoints[39], waypoints[41]);
    wayPointSystem.insertTwoSided(waypoints[41], waypoints[42]);
    wayPointSystem.insertTwoSided(waypoints[42], waypoints[43]);
    wayPointSystem.insertTwoSided(waypoints[44], waypoints[45]);
    wayPointSystem.insertTwoSided(waypoints[45], waypoints[46]);
    wayPointSystem.insertTwoSided(waypoints[46], waypoints[47]);
    wayPointSystem.insertTwoSided(waypoints[47], waypoints[48]);
    wayPointSystem.insertTwoSided(waypoints[48], waypoints[49]);
    wayPointSystem.insertTwoSided(waypoints[50], waypoints[51]);
    wayPointSystem.insertTwoSided(waypoints[51], waypoints[53]);
    wayPointSystem.insertTwoSided(waypoints[52], waypoints[53]);
    wayPointSystem.insertTwoSided(waypoints[52], waypoints[54]);
    wayPointSystem.insertTwoSided(waypoints[55], waypoints[56]);
    wayPointSystem.insertTwoSided(waypoints[56], waypoints[57]);
    wayPointSystem.insertTwoSided(waypoints[58], waypoints[59]);
    wayPointSystem.insertTwoSided(waypoints[59], waypoints[60]);
    wayPointSystem.insertTwoSided(waypoints[60], waypoints[61]);
    wayPointSystem.insertTwoSided(waypoints[61], waypoints[62]);
    wayPointSystem.insertTwoSided(waypoints[62], waypoints[63]);
    wayPointSystem.insertTwoSided(waypoints[63], waypoints[64]);
    wayPointSystem.insertTwoSided(waypoints[64], waypoints[65]);
    wayPointSystem.insertTwoSided(waypoints[65], waypoints[66]);
    wayPointSystem.insertTwoSided(waypoints[66], waypoints[67]);
    wayPointSystem.insertTwoSided(waypoints[5], waypoints[9 ]);

    m_->scene->updateGraphics();

    for (size_t i = 0; i < GamePrivate::numPersons; ++i)
    {
        m_->persons[i] = std::make_shared<Person>(GamePrivate::personsNames[i]);
        m_->scene->attachObject(m_->persons[i]);

        float angle = 2.0f * glm::pi<float>() * i / GamePrivate::numPersons;
        m_->persons[i]->graphicsNode()->setTransform(utils::Transform(
                                                         glm::vec3(1.f,1.f,1.f),
                                                         glm::quat_cast(glm::mat3x3(glm::vec3(-.7f,0.f,-.7f), glm::vec3(0.f,1.f,0.f), glm::vec3(.7f,0.f,-.7f))),
                                                         20.0f * glm::vec3(glm::cos(angle), 0.0f,glm::sin(angle))));

        m_->acivePerson = m_->persons[i];
    }

    static const std::vector<glm::vec3> lightColors {
        glm::vec3(1,0,0),
                glm::vec3(0,1,0),
                glm::vec3(0,0,1),
                glm::vec3(1,1,0),
                glm::vec3(1,0,1),
                glm::vec3(0,1,1),
    };

    for (int z = 0; z < 10; ++z)
    {
        auto l = std::make_shared<core::Light>(core::LightType::Point);
        l->setPosition(glm::vec3(rnd(-2500, 2500), 1000.0f, rnd(-2500, 2500)));
        l->setDirection(glm::vec3(0.0,-1.0,0.0));
        l->setSpotAngles(glm::vec2(1.0f, 1.7f));
        l->setColor(3.0f * lightColors.at(rand() % lightColors.size()));
        l->setRadiuses(glm::vec2(1200.0f, 100.0f));
        //l->enableShadowMap(false);
        //m_->scene->scene()->attachLight(l);

        lights.push_back(l);
        lightsPos.push_back(l->position());
        lightsCW.push_back(rand()%2 ? 1.f : -1.f);
        lightsR.push_back(rnd(500.f, 1000.f));
    }

    auto l = std::make_shared<core::Light>(core::LightType::Direction);
    auto d = glm::vec3(-5.0f, 15.0f, 5.0f);
    l->setPosition(d);
    l->setDirection(-d);
    l->setColor(glm::vec3(0.5f, 0.5f, 0.5f)*3.f);
    l->setSpotAngles(glm::vec2(1.0f, 40.0f));
    m_->scene->graphicsScene()->attachLight(l);

    m_->scene->graphicsScene()->setProjectionMatrixAsPerspective(glm::pi<float>() * 0.25f);
    m_->scene->graphicsScene()->setViewMatrix(glm::lookAt(glm::vec3(.0f, 0.f, 0.f), glm::vec3(.0f, 0.f, -1.f), glm::vec3(0.0f, 1.0f, 0.0f)));


    const size_t wpFrom = 2;
    auto person = m_->persons[0];
    auto transform = person->graphicsNode()->transform();
    transform.translation = waypoints[wpFrom]->position;
    person->graphicsNode()->setTransform(transform);
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

    const float r = 2.2f;
    const float t = /*3.14f / 4;*/time * 0.00001f;
    m_->scene->graphicsScene()->setViewMatrix(glm::lookAt(glm::vec3(11 * r * cos(t), 10 * r, -11 * r * sin(t)), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f)));

    for (int i = 0; i < lights.size(); ++i)
    {

        //lights[i]->setPosition(lightsPos[i] + lightsR[i] * glm::vec3(cos(10*t)*lightsCW[i], 0, sin(10*t)));
    }

    for (auto person: m_->persons)
    {
        auto personNode = person->graphicsNode();
        utils::Ray ray = personNode->globalTransform() * utils::Ray(personNode->boundingBox().center(), glm::vec3(0.f, 0.f, 1.f));

        auto transform = personNode->globalTransform() *
                glm::translate(glm::mat4x4(1.0f), personNode->boundingBox().center()) *
                glm::rotate(glm::mat4x4(1.0f), glm::pi<float>(), glm::vec3(0.f, 1.f, 0.f));

        utils::Frustum frustum(glm::perspective(glm::pi<float>()*0.25f, 1.0f, .5f, 3000.0f) * glm::inverse(transform));

        core::NodeFrustumIntersectionVisitor nv(frustum);
        m_->scene->graphicsScene()->rootNode()->accept(nv);
        const auto& intersectionData = nv.intersectionData();

        person->setText(person->name());

        bool isFound = false;
        for (const auto& intersection: intersectionData.nodes)
        {
            if (intersection.first < 0.f)
                continue;

            auto intersectedObject = m_->scene->findObject(intersection.second);
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
    utils::Ray ray = m_->scene->graphicsScene()->throwRay(x, y);

    core::NodeRayIntersectionVisitor nv(ray);
    m_->scene->graphicsScene()->rootNode()->accept(nv);
    const auto& intersectionData = nv.intersectionData();

    if (!intersectionData.nodes.empty())
    {
        auto firstDist = intersectionData.nodes.begin()->first;
        auto firstNode = intersectionData.nodes.begin()->second;

        auto k = firstNode->relationDegree(m_->scene->floorNode());
        if (k != -1 && !m_->acivePerson.expired())
        {
            const glm::vec3 clickCoord = ray.calculatePoint(firstDist);
            const glm::vec3 personPos = m_->acivePerson.lock()->graphicsNode()->globalTransform().translation;

            static const float shift = 0.25f;
            auto path = m_->scene->buildRoute(personPos, clickCoord, {glm::vec2(0.f, 0.75f - shift),
                                                                      glm::vec2(0.f - shift, 0.75f + shift),
                                                                      glm::vec2(0.f + shift, 0.75f + shift)});

            auto person = m_->acivePerson.lock();
            person->clearTasks();
            for (auto wp : path)
                person->addTask(std::make_shared<PersonTaskRun>(wp->position));
            person->addTask(std::make_shared<PersonTaskRun>(clickCoord));
        }
        else
        {
            auto object = Scene::findObject(firstNode);
            for (size_t i = 0; i < GamePrivate::numPersons; ++i)
            {
                if (m_->persons[i] == object)
                {
                    m_->acivePerson = m_->persons[i];
                    m_->acivePerson.lock()->addTask(std::make_shared<PersonTaskWave>());
                    break;
                }
            }
        }
    }
}

} // namespace
} // namespace
