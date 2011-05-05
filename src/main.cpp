#include <QApplication>

#include "MPlotWidget.h"
#include "MPlotSeriesData.h"
#include "MPlotSeries.h"

#include "MPlotImageData.h"
#include "MPlotImage.h"

#include "MPlotPoint.h"

#include <QTableView>
#include <QPen>
#include <QBrush>

#include <QGradientStops>
#include <QGradientStop>

#include <QPrinter>
#include <QPrintDialog>

// #include <QGLWidget>
#include <cmath>

#include "MPlotTools.h"


 int main(int argc, char *argv[])
 {

	QApplication app(argc, argv);


	 // 1. Creating Plots:
	 ////////////////////////////

	 // An MPlotWidget is needed to view a plot:
	 MPlotWidget plotWindow;

	 // Enable or disable anti-aliasing. (The drawing engine on Mac OS X creates double-width lines for some window sizes, unless anti-aliasing is turned on.)
	 plotWindow.enableAntiAliasing(true);

	 // An MPlot is the QGraphicsItem representing a 2D Plot:
	MPlot plot;
	plotWindow.setPlot(&plot);

	 // 2. Configuring Axis settings (Look and style; placement and number of tick marks)
	 //////////////////////////////////



	 //plot.axisTop()->setTickPen(QPen(QBrush(QColor(Qt::yellow)), 0));
	 //plot.axisBottom()->setTickPen(QPen(QBrush(QColor(Qt::red)), 0));
	 // plot.axisLeft()->setTickPen(QPen(QBrush(QColor(Qt::green)), 0));
	 //plot.axisRight()->setTickPen(QPen(QBrush(QColor(Qt::blue)), 0));
	 plot.axisRight()->setTicks(3, MPlotAxis::Middle, 5);	// Set the approximate number and style of axis tick marks:
	 plot.axisRight()->showTickLabels(true);
	 plot.axisRight()->setAxisName("y right");
	 plot.axisRight()->showAxisName(true);

	 plot.axisBottom()->setAxisName("eV");
	 plot.axisLeft()->setAxisName("Intensity (arb. units)");



	 // Change the margins: (in % of the plot width/height)
	 plot.setMarginTop(5);
	 plot.setMarginRight(10);
	 plot.setMarginLeft(15);
	 plot.setMarginBottom(15);


	 // Show(hide) an axis completely
	 //plot.axisRight()->setVisible(true);

	 // Show ticks but not value labels:
	 //plot.axisRight()->showTickLabels(false);

	 // Disable tick marks completely:
	 plot.axisTop()->setTicks(0);
	 plot.axisTop()->setAxisName("time (s)");
	 plot.axisTop()->showAxisName();

	// plot.axisTop()->showTickLabels(true);


	 // 13: Adding 2d data and Image plots:
//	 MPlotSimpleImageData data2d(QRectF(-0.5,-0.5,1,1), QSize(1024,1024));
//	 for(int yy=0; yy<1024; yy++) {
//		 for(int xx=0; xx<1024; xx++) {
//			 qreal x = data2d.x(xx);
//			 qreal y = data2d.y(yy);
//			 //qreal r2 = x*x + y*y;
//			 //data2d.setZ(exp(-r2/0.1), xx, yy);
//			 data2d.setZ(sin(x*4*M_PI)*sin(y*2*M_PI), xx, yy);
//		 }
//	 }

//	 MPlotImageBasic* plot2d = new MPlotImageBasic(&data2d);
//	 plot2d->setColorMap(MPlotColorMap::Jet);	//This should be the default color map.
//	 plot.addItem(plot2d);


	 // 3. Add data. Data is contained in the first two columns of an MPlotSeriesData:
	 //////////////////////////////
	 MPlotRealtimeModel data1, data2;



	 /*
	 data1.insertPointBack(0.55, 0.57);
	 data1.insertPointFront(0.4, 0.43);
	 data1.insertPointBack(0.6, 0.68);
	 data1.insertPointBack(-0.1, -0.1);
	 data1.insertPointBack(-0.2, -0.2);
	 data1.insertPointBack(-0.3, -0.3);
	 data1.insertPointBack(0, 0);
	 data1.insertPointBack(10, 10);
	  */


	 // Fill with parabola:
	 for(qreal i=-.9; i<.99; i+=0.01)
		 data1.insertPointBack(i, -i*i+0.5);

	 // Fill with many random data points:
	  //for(int i=0; i<5000; i++)
		// data2.insertPointBack(qreal(rand())/RAND_MAX/2, qreal(rand())/RAND_MAX/2);

	 // many-point sine wave:
	 for(int i=0; i<9990; i++)
		 data2.insertPointBack(-0.5+i/9990.0, sin((-0.5+i/9990.0)*4*3.1415));


	 // 4.  View the data.  A basic scatter/line plot is an MPlotSeries:
	 ////////////////////////////////////////////////////
	 MPlotSeriesBasic* series1 = new MPlotSeriesBasic();
	 MPlotSeriesBasic* series2 = new MPlotSeriesBasic();
	 //series1->setName("series1");
	 //series2->setName("series2");

	 // Set to plot on the right axis instead of the left axis
	 // series1->setYAxisTarget(MPlotAxis::Right);

	 // connect this plot series as a view on its model (data1, data2)
	 series1->setModel(&data1);
	 series2->setModel(&data2);


	 // 5. Configure look of the plots:
	 //////////////////////////////////////
	 QPen redSkinny(QBrush(QColor(Qt::red)), 1);	// red, 1pts wide
	 QPen greenFat(QBrush(QColor(Qt::green)), 2);
	 QPen pinkSkinny(QBrush(QColor(Qt::magenta)), 0);

	 // Line style: set using pens.  (Can create dashed pen for dashed/dotted lines)
	 series1->setLinePen( redSkinny);	// set the pen for drawing the series
	 series2->setLinePen( greenFat );

	 // Marker size and shape
	 // series2->setMarker(MPlotMarkerShape::StarCircle, 6);
	 series2->setMarker(MPlotMarkerShape::None);

	 // Can also configure the marker pen and brush: (only can do this when MPlotMarkerShape != None)
//	 series2->marker()->setPen(pinkSkinny);
//	 series2->marker()->setBrush(QBrush(QColor(Qt::black)));

	 // Gridlines:
	 //plot.axisRight()->showGrid(true);
	 //plot.axisRight()->setGridPen(greenFat);


	 // 6. Adding a series to a plot:
	 ///////////////////////////////
	 series1->setDescription("series 1");
	 plot.addItem(series1);
	 //plot.addItem(series2);

	 // 2. (continued) Axis / Axis Scale Settings
	 ///////////////////////

	 // How much scale padding to add around data (in percent)
	 //plot.setScalePadding(5);	// set axis scale padding in percent

	 // Manual axis range:
	 //plot.setXDataRange(-1.5, 1.5);		// Manually set the axis range
	 //plot.setYDataRangeLeft(-0.5, 0.5);

	 // To auto-scale once only (using the current data):
	 // plot.setXDataRange(0, 0, true);
	 // plot.setYDataRangeLeft(0, 0, true);

	 // Auto-scale always (ie: rescale as new data arrives)
	 plot.axisScaleLeft()->setAutoScaleEnabled();
	 plot.axisScaleBottom()->setAutoScaleEnabled();

	 // 7. Testing adding points to the series after the series is created.
	 //////////////////////////////////////
	 //data2.insertPointBack(0, 0);
	 //data2.insertPointBack(0.2, 0);
	 //data2.insertPointFront(0, 0.2);
	 //data2.removePointBack();
	// data2.removePointFront();

	 // For real-time data: most optimized for memory consumption if you always do "insertPointBack()" and "removePointFront()".



	 // 8. Data-point editor. Standard QTableViews can also act as views on the MPlotSeriesData model.
	 //////////////////////////////
	 QTableView editor;
	 editor.setModel(&data1);


	 // 9. Display UI:
	 //////////////////////
	 editor.resize(300, 400);
	 editor.show();

	 plotWindow.resize(400, 300);
	 plotWindow.show();

	// 5. (continued) More fun with marker shapes... Testing changes after a plot is created:
	//////////////////
	series1->setMarker(MPlotMarkerShape::Cross, 12, QPen(QColor(Qt::yellow), 0));


	// 10: single markers:
	MPlotPoint p1;
	p1.setValue(QPointF(0.5,0.5));
	p1.setMarker(MPlotMarkerShape::CrossCircle, 24, QPen(QColor(Qt::red)));

	// plot.addItem(&p1);


	 // 11. Enable, disable, and selection?
	 /////////////////////////////
	// series2->setEnabled(true);
	// series2->setSelected(true);

	 // 10. Printing:
	 ////////////////////


/*
	 QPrinter printer;
	 printer.setOrientation(QPrinter::Landscape);
	 if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
		 QPainter painter(&printer);
		 painter.setRenderHint(QPainter::Antialiasing);
		 plotWindow.scene()->render(&painter);
	 } // Print this to a PDF to see vector-graphics export.
*/

	 // PNG export:
/*
	QPixmap pixmap(800, 600);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	plotWindow.scene()->render(&painter);
	painter.end();

	pixmap.save("/Users/mboots/Desktop/scene.png");
*/


	 // Try out openGl viewport: (instead of CoreGraphics on Mac OS X)
	 //plotWindow.setViewport(new QGLWidget);

	// 12: Testing plot tools: MPlotPlotSelectorTool:


	// DragZoomerTools need to be added first ("on the bottom") so they don't steal everyone else's mouse events
	MPlotDragZoomerTool dzTool;
	plot.addTool(&dzTool);
	// dzTool.setEnabled(false);

	// this tool selects a plot with the mouse
	MPlotPlotSelectorTool psTool;
	plot.addTool(&psTool);
	// psTool.setEnabled(false);

	// this tool adds mouse-wheel based zooming
	MPlotWheelZoomerTool wzTool;
	plot.addTool(&wzTool);

	// this tool adds a cursor (or more) to a plot
//	MPlotCursorTool crsrTool;
//	plot.addTool(&crsrTool);
//	crsrTool.addCursor(plot.axisScaleLeft(), plot.axisScaleBottom());
//	crsrTool.cursor(0)->marker()->setPen(QPen(QColor(Qt::blue)));


	return app.exec();
 }
