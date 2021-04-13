#include <QtGui/QOpenGLContext>
#include <QtGui/QMouseEvent>
#include <QtGui/QCloseEvent>
#include <QtGui/QStaticText>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>

#include <core/core.h>

#include "renderwidget.h"
#include "renderer.h"
#include "importexport.h"

namespace trash
{
namespace core
{

RenderWidget::RenderWidget(Core& core)
    : QOpenGLWidget()
    , m_core(core)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

RenderWidget::~RenderWidget()
{
}

const Renderer &RenderWidget::renderer() const
{
    return *m_renderer;
}

Renderer &RenderWidget::renderer()
{
    return *m_renderer;
}

void RenderWidget::initializeGL()
{
    m_renderer = std::make_unique<Renderer>(*context()->extraFunctions(), defaultFramebufferObject());
    m_renderer->initialize();

    m_timer = std::make_unique<QTimer>(this);
    connect(m_timer.get(), SIGNAL(timeout()), SLOT(update()));
    m_timer->setInterval(0);
    m_timer->start();

    m_startTime = m_lastFpsTime = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
    m_lastUpdateTime = 0;

    m_core.sendMessage(std::make_shared<RenderWidgetWasInitializedMessage>());
}

void RenderWidget::resizeGL(int w, int h)
{
    m_renderer->resize(w, h);
}

void RenderWidget::paintGL()
{
    uint64_t time = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch()) - m_startTime;
    uint64_t dt = time - m_lastUpdateTime;
    m_lastUpdateTime = time;

    static const uint64_t deltaFps = 1000;
    ++m_fpsCounter;
    if (time - m_lastFpsTime >= deltaFps)
    {
        m_lastFps = m_fpsCounter / (0.001f * deltaFps);
        m_fpsCounter = 0;
        m_lastFpsTime = time;
    }

    m_core.sendMessage(std::make_shared<RenderWidgetWasUpdatedMessage>(time, dt));
    m_core.process();

    int textSize = static_cast<int>(static_cast<float>(height()) / 720 * 28);
    int textXY = static_cast<int>(static_cast<float>(height()) / 720 * 10);

    QPainter painter(this);
    painter.setPen(Qt::red);
    painter.setFont(QFont("Arial", textSize));
    painter.drawStaticText(QPoint(textXY, textXY), QStaticText("FPS: " + QString::number(static_cast<double>(m_lastFps), 'f', 1)));
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    m_core.sendMessage(std::make_shared<RenderWidgetMouseClickMessage>(mouseButtonMask(event->buttons()), event->x(), event->y()));
    event->accept();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_core.sendMessage(std::make_shared<RenderWidgetMouseMoveMessage>(event->buttons(), event->x(), event->y()));
    event->accept();
}

void RenderWidget::closeEvent(QCloseEvent *event)
{
    m_core.sendMessage(std::make_shared<RenderWidgetWasClosedMessage>());
    m_core.process();
    event->accept();
}

uint32_t RenderWidget::mouseButtonMask(const Qt::MouseButtons& qtMask)
{
    uint32_t result =
            (qtMask.testFlag(Qt::MouseButton::LeftButton) ? MouseButton::LeftButton : 0) |
            (qtMask.testFlag(Qt::MouseButton::MiddleButton) ? MouseButton::MiddleButton : 0) |
            (qtMask.testFlag(Qt::MouseButton::RightButton) ? MouseButton::RightButton : 0);
    return result;
}

} // namespace
} // namespace
