#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QtWidgets/QOpenGLWidget>

#include <core/coreglobal.h>

class CORESHARED_EXPORT RenderWidget : public QOpenGLWidget
{
public:
    explicit RenderWidget();
    ~RenderWidget() override;

protected:
    void initializeGL() override;
    void resizeGL(int, int) override;
    void paintGL() override;

};

#endif // RENDERWIDGET_H
