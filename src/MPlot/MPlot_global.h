#ifndef MPLOT_GLOBAL_H
#define MPLOT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MPLOT_LIBRARY)
#  define MPLOTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MPLOTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MPLOT_GLOBAL_H
