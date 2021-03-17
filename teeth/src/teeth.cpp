#include <glm/gtc/matrix_transform.hpp>

#include <utils/boundingbox.h>
#include <utils/transform.h>

#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/scene.h>
#include <core/modelnode.h>
#include <core/scenerootnode.h>

#include <teeth/teeth.h>


namespace trash
{
namespace teeth
{

Teeth::Teeth()
{
}

Teeth::~Teeth()
{

}

void Teeth::doInitialize()
{
    m_grpahicsScene = std::make_shared<core::Scene>();
    core::Core::instance().graphicsController().setMainScene(m_grpahicsScene);

    m_grpahicsScene->setProjectionMatrixAsPerspective(glm::pi<float>() * 0.1f);
    m_grpahicsScene->setViewMatrix(glm::lookAt(glm::vec3(0.f, 0.f, 3.f), glm::vec3(.0f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f)));

    auto teethNode = std::make_shared<core::ModelNode>("teeth.mdl");
    m_grpahicsScene->rootNode()->attach(teethNode);

    teethNode->detach(teethNode->children().at(1));

    teethNode->setTransform(utils::Transform::fromScale(10.f));
}

void Teeth::doUnitialize()
{
    m_grpahicsScene = nullptr;
}

void Teeth::doUpdate(uint64_t, uint64_t)
{

}

void Teeth::doMouseClick(uint32_t buttonMask, int x, int y)
{
    m_mouseX = x;
    m_mouseY = y;
}

void Teeth::doMouseMove(uint32_t buttonMask, int x, int y)
{
    int deltaX = x - m_mouseX;
    int deltaY = y - m_mouseY;
    m_mouseX = x;
    m_mouseY = y;

    if (buttonMask & core::MouseButton::LeftButton)
    {
        m_modelAngleX += deltaX;
        while (m_modelAngleX >= 360.f) m_modelAngleX -= 360.f;
        while (m_modelAngleX < 0.f) m_modelAngleX += 360.f;

        m_modelAngleY += deltaY;
        if (m_modelAngleY > 90.f) m_modelAngleY = 90.f;
        if (m_modelAngleY < -90.f) m_modelAngleY = -90.f;

        glm::mat4x4 rotateMatrix =
                glm::rotate(glm::mat4x4(1.0f), glm::radians(m_modelAngleY), glm::vec3(1.f, 0.f, 0.f)) *
                glm::rotate(glm::mat4x4(1.0f), glm::radians(m_modelAngleX), glm::vec3(0.f, 1.f, 0.f))
                ;

        m_grpahicsScene->rootNode()->setTransform(
                    utils::Transform::fromRotation(glm::quat_cast(rotateMatrix))
                    );
    }
}

} // namespace
} // namespace
