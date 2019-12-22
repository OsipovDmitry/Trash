#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>

#include <core/core.h>
#include <game/game.h>

#include <iostream>

int main(int argc, char *argv[])
{
    static const int widgetHeight = 200;

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

    auto game = std::make_shared<trash::game::Game>();
    trash::core::Core::instance().setGame(game);
    trash::core::Core::instance().renderWidget().resize(widgetHeight * 16 / 9, widgetHeight);
    trash::core::Core::instance().renderWidget().show();

    int res = QApplication::exec();
    std::cout << "finish!" << std::endl;
    return res;
}
