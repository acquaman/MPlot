#-------------------------------------------------
#
# QMake Project for building an MPlot demo app
#
#-------------------------------------------------

TEMPLATE = app
TARGET = MPlotTest
DEPENDPATH += . \
	src \
	src/MPlot

INCLUDEPATH += include

equals(QMAKE_CXX, "clang++"){
	DEFINES *= MPLOT_PRAGMA_WARNING_CONTROLS
}

MPLOTLIBPATH = $${PWD}/lib
LIBS += -L$${MPLOTLIBPATH} -lMPlot

# Input
HEADERS +=

SOURCES += src/main.cpp
