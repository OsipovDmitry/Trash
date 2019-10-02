#ifndef GAMEGLOBAL_H
#define GAMEGLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GAME_LIBRARY)
#  define GAMESHARED_EXPORT Q_DECL_EXPORT
#else
#  define GAMESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GAMEGLOBAL_H
