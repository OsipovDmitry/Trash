#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <memory>

#include <QtWidgets/QOpenGLWidget>

#include <utils/message.h>

#include <core/abstractcontroller.h>
#include <core/types.h>

class QTimer;
class Renderer;
class Core;

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
    void closeEvent(QCloseEvent*) override;

private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<QTimer> m_timer;
    Core& m_core;
    uint64_t m_startTime, m_lastUpdateTime, m_lastFpsTime;
    uint32_t m_fpsCounter;
    float m_lastFps;
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

class RenderWidgetWasClickedMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::RenderWidgetWasClicked)
public:
    int x, y;
    RenderWidgetWasClickedMessage(int xPos, int yPos) : AbstractController::Message(messageType()), x(xPos), y(yPos) {}
};

class RenderWidgetWasClosedMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::RenderWidgetWasClosed)
public:
    RenderWidgetWasClosedMessage() : AbstractController::Message(messageType()) {}
};

#endif // RENDERWIDGET_H
