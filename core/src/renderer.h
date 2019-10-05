#ifndef RENDERER_H
#define RENDERER_H

#include <gl/GL.h>
#include <string>

#include <core/types.h>

class QOpenGLExtraFunctions;
class Renderer;

class Renderer
{
public:
    Renderer(QOpenGLExtraFunctions&);
    ~Renderer();

    void resize(int, int);
    void render();

private:
    QOpenGLExtraFunctions& m_functions;
};


#endif // RENDERER_H
