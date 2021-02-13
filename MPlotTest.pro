#-------------------------------------------------
#
# QMake Project for building an MPlot demo app
#
#-------------------------------------------------

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TEMPLATE = app
TARGET = MPlotTest

CONFIG += depend_includepath
DEPENDPATH += . \
	src \
	src/MPlot

INCLUDEPATH += include

equals(QMAKE_CXX, "clang++"){
	DEFINES *= MPLOT_PRAGMA_WARNING_CONTROLS
}

# Set standard level of compiler warnings for everyone. (Otherwise the warnings shown will be system-dependent.)
QMAKE_CXXFLAGS *= -Wextra -g

MPLOTLIBPATH = $${PWD}/lib
LIBS += -L$${MPLOTLIBPATH} -lMPlot

# Input
HEADERS +=

SOURCES += src/main.cpp
