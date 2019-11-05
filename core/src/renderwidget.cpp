#include <QtGui/QOpenGLContext>
#include <QtGui/QMouseEvent>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>

#include <core/core.h>

#include "renderwidget.h"
#include "renderer.h"

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
    m_renderer = std::make_unique<Renderer>(*context()->extraFunctions());
    m_renderer->initializeResources();

    m_timer = std::make_unique<QTimer>(this);
    connect(m_timer.get(), SIGNAL(timeout()), SLOT(update()));
    m_timer->setInterval(16);
    m_timer->start();

    m_startTime = m_lastUpdateTime = m_lastFpsTime = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());

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

    static const uint64_t deltaFps = 300;
    ++m_fpsCounter;
    if (time - m_lastFpsTime >= deltaFps)
    {
        m_lastFps = m_fpsCounter / (0.001f * deltaFps);
        m_fpsCounter = 0;
        m_lastFpsTime = time;
    }

    m_core.sendMessage(std::make_shared<RenderWidgetWasUpdatedMessage>(time, dt));
    m_core.process();

    m_renderer->render();

    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 16));
    painter.drawText(10, 10, width(), height(), Qt::AlignTop | Qt::AlignLeft, "FPS: " + QString::number(static_cast<double>(m_lastFps), 'f', 1));
    painter.end();
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    m_core.sendMessage(std::make_shared<RenderWidgetWasClickedMessage>(event->x(), event->y()));
    event->accept();
}

void RenderWidget::closeEvent(QCloseEvent *event)
{
    m_core.sendMessage(std::make_shared<RenderWidgetWasClosedMessage>());
    m_core.process();
    event->accept();
}
