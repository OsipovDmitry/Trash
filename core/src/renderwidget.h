#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <memory>

#include <QtWidgets/QOpenGLWidget>

#include <utils/message.h>

#include <core/forwarddecl.h>
#include <core/abstractcontroller.h>
#include <core/types.h>

#include "typesprivate.h"

class QTimer;

namespace trash
{
namespace core
{

class Renderer;

class RenderWidget : public QOpenGLWidget
{
public:
    explicit RenderWidget(Core&);
    ~RenderWidget() override;

    const Renderer& renderer() const;
    Renderer& renderer();

protected:
    void initializeGL() override;
    void resizeGL(int, int) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void closeEvent(QCloseEvent*) override;

private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<QTimer> m_timer;
    Core& m_core;
    uint64_t m_startTime, m_lastUpdateTime, m_lastFpsTime;
    uint32_t m_fpsCounter;
    float m_lastFps;

    static uint32_t mouseButtonMask(const Qt::MouseButtons&);
};

class RenderWidgetWasInitializedMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::RenderWidgetWasInitialized)
public:
    RenderWidgetWasInitializedMessage() : AbstractController::Message(messageType()) {}
};

class RenderWidgetWasUpdatedMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::RenderWidgetWasUpdated)
public:
    uint64_t time, dt;
    RenderWidgetWasUpdatedMessage(uint64_t time_, uint64_t dt_) : AbstractController::Message(messageType()), time(time_), dt(dt_) {}
};

class RenderWidgetMouseClickMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::RenderWidgetMouseClick)
public:
    int x, y;
    uint32_t buttonMask;
    RenderWidgetMouseClickMessage(uint32_t bmask, int xPos, int yPos) : AbstractController::Message(messageType()), buttonMask(bmask), x(xPos), y(yPos) {}
};

class RenderWidgetMouseMoveMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::RenderWidgetMouseMove)
public:
    int x, y;
    uint32_t buttonMask;
    RenderWidgetMouseMoveMessage(uint32_t bmask, int xPos, int yPos) : AbstractController::Message(messageType()), buttonMask(bmask), x(xPos), y(yPos) {}
};

class RenderWidgetWasClosedMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::RenderWidgetWasClosed)
public:
    RenderWidgetWasClosedMessage() : AbstractController::Message(messageType()) {}
};

} // namespace
} // namespace

#endif // RENDERWIDGET_H
