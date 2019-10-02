#include <QtWidgets/QApplication>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>

#include <core/renderwidget.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat surfaceFormat;
    surfaceFormat.setDepthBufferSize(24);
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        surfaceFormat.setVersion(3, 3);
        surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    }
    else
    {
        surfaceFormat.setVersion(3, 0);
    }
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    RenderWidget renderWidget;
    renderWidget.show();

    int res = QApplication::exec();
    return res;
}
