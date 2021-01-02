#ifndef TEETHGLOBAL_H
#define TEETHGLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TEETH_LIBRARY)
#  define TEETHSHARED_EXPORT Q_DECL_EXPORT
#else
#  define TEETHSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // TEETHGLOBAL_H
