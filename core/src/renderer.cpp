#include <QtGui/QOpenGLExtraFunctions>

#include "renderer.h"


Renderer::Renderer(QOpenGLExtraFunctions& functions)
    : m_functions(functions)
{
    m_functions.glClearColor(.5f, .5f, 1.f, 1.f);
}

Renderer::~Renderer()
{
}

void Renderer::resize(int width, int height)
{
    m_functions.glViewport(0, 0, width, height);
}

void Renderer::render()
{
    m_functions.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
