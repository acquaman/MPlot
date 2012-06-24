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

INCLUDEPATH += ./include

MPLOTLIBPATH = $${PWD}/lib
LIBS += -L$${MPLOTLIBPATH} -lMPlot

# Input
HEADERS +=

SOURCES += src/main.cpp
