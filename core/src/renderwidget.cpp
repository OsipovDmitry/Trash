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

    m_timer = std::make_unique<QTimer>(this);
    connect(m_timer.get(), SIGNAL(timeout()), SLOT(update()));
    m_timer->setInterval(16);
    m_timer->start();

    m_startTime = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
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

    m_core.sendMessage(std::make_shared<RenderWidgetWasUpdatedMessage>(time, dt));
    m_core.process();

    m_renderer->render();
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
