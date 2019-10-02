#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLExtraFunctions>

#include <core/renderwidget.h>

RenderWidget::RenderWidget()
    : QOpenGLWidget()
{
}

RenderWidget::~RenderWidget()
{
}

void RenderWidget::initializeGL()
{
    context()->extraFunctions()->glClearColor(.5f, .5f, 1.f, 1.f);
}

void RenderWidget::resizeGL(int w, int h)
{
    context()->extraFunctions()->glViewport(0, 0, w, h);
}

void RenderWidget::paintGL()
{
    context()->extraFunctions()->glClear(GL_COLOR_BUFFER_BIT);
}

