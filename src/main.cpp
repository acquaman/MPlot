#include <QApplication>

#include "MPlotWindow.h"
#include "MPlotSeriesData.h"
#include <QTableView>
#include <QPen>
#include <QBrush>

#include <QPrinter>
#include <QPrintDialog>

 int main(int argc, char *argv[])
 {

	QApplication app(argc, argv);


	 // 1. Creating Plots:
	 ////////////////////////////
	 
	 // An MPlotWindow is needed to view a plot:
	 MPlotWindow plotWindow;
	 
	 // An MPlot is the QGraphicsView representing a 2D Plot:
	 MPlot plot;
	 // Connecting the window to view the plot:
	 plotWindow.setPlot(&plot);	
	 
	 // 2. Configuring Axis settings (Look and style; placement and number of tick marks)
	 //////////////////////////////////
	 
	 //plot.axisTop()->setTickPen(QPen(QBrush(QColor(Qt::yellow)), 0));
	 //plot.axisBottom()->setTickPen(QPen(QBrush(QColor(Qt::red)), 0));
	 //plot.axisLeft()->setTickPen(QPen(QBrush(QColor(Qt::green)), 0));
	 //plot.axisRight()->setTickPen(QPen(QBrush(QColor(Qt::blue)), 0));
	 plot.axisRight()->setTicks(3, MPlotAxis::Inside);	// Set the approximate number and style of axis tick marks:
	 
	 // Change the margins: (in % of the plot width/height)
	 plot.setMarginTop(5);
	 plot.setMarginRight(5);
	 plot.setMarginLeft(15);
	 plot.setMarginBottom(15);

	 
	 // Show(hide) an axis completely
	 //plot.axisRight()->setVisible(true);
	 
	 // Show ticks but not value labels:
	 //plot.axisRight()->showTickLabels(false);
	 
	 // Disable tick marks completely:
	 plot.axisTop()->setTicks(0);
	 

	 // 3. Add data. Data is contained in the first two columns of an MPlotSeriesData:
	 //////////////////////////////
	 MPlotSeriesData data1, data2;
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
	 for(double i=-.9; i<.99; i+=0.01)
		 data1.insertPointBack(i, -i*i+0.5);
	 
	 // Fill with many random data points:
	 // for(int i=0; i<1000; i++)
	 //	 data2.insertPointBack(double(rand())/RAND_MAX/2, double(rand())/RAND_MAX/2);

	 
	 // 4.  View the data.  A basic scatter/line plot is an MPlotSeries:
	 ////////////////////////////////////////////////////
	 MPlotSeries series1, series2;
	 series1.setObjectName("series1");
	 series2.setObjectName("series2");
	 
	 // Enable to plot on the right axis instead of the left axis
	 // series1.setYAxisTarget(MPlotAxis::Right);
	 
	 // connect this plot series as a view on its model (data1, data2)
	 series1.setModel(&data1);	
	 series2.setModel(&data2);
	 
	 
	 // 5. Configure look of the plots:
	 //////////////////////////////////////
	 QPen redSkinny(QBrush(QColor(Qt::red)), 1);	// red, 1pts wide
	 QPen greenFat(QBrush(QColor(Qt::green)), 2);
	 QPen pinkSkinny(QBrush(QColor(Qt::magenta)), 0);
	 
	 // Line style: set using pens.  (Can create dashed pen for dashed/dotted lines)
	 series1.setLinePen( redSkinny);	// set the pen for drawing the series
	 series2.setLinePen( greenFat );
	 
	 // Marker size and shape:
	 series2.setMarkerSize(12);
	 series2.setMarkerShape(MPlotMarkerShape::StarCircle);
	 
	 // Can also configure the marker pen and brush:
	 series2.setMarkerPen(pinkSkinny);
	 series2.setMarkerBrush(QBrush(QColor(Qt::black)));
	 
	 
	 // 6. Adding a series to a plot:
	 ///////////////////////////////
	 plot.addSeries(&series1);
	 //plot.addSeries(&series2);
	 
	 // 2. (continued) Axis / Axis Scale Settings
	 ///////////////////////
	 
	 // How much scale padding to add around data (in percent)
	 plot.setScalePadding(5);	// set axis scale padding in percent

	 // Manual axis range:
	 plot.setXDataRange(-1.5, 1.5);		// Manually set the axis range
	 plot.setYDataRangeLeft(-0.5, 0.5);
	 
	 // To auto-scale once only (using the current data):
	 // plot.setXDataRange(0, 0, true);
	 // plot.setYDataRangeLeft(0, 0, true);
	 
	 // Auto-scale always (ie: rescale as new data arrives)
	 // plot.enableAutoScale(MPlotAxis::Left | MPlotAxis::Bottom);
	 
	 // 7. Testing adding points to the series after the series is created.
	 //////////////////////////////////////
	 data2.insertPointBack(0, 0);
	 data2.insertPointBack(0.2, 0);
	 data2.insertPointFront(0, 0.2);
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
	series1.setMarkerSize(6);
	series1.setMarkerShape(MPlotMarkerShape::Cross);
	 
	 // 11. Enable, disable, and selection?
	 /////////////////////////////
	// series2.setEnabled(true);
	// series2.setSelected(true);
	 
	 // 10. Printing:
	 ////////////////////
	 
	 /*
	 QPrinter printer;
	 printer.setOrientation(QPrinter::Landscape);
	 if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
		 QPainter painter(&printer);
		 painter.setRenderHint(QPainter::Antialiasing);
		 plot.render(&painter);
	 } // Print this to a PDF to see vector-graphics export.  Wow that was easy!
	  */
	 
	 /* PNG export:
	 QPixmap pixmap;
	 QPainter painter(&pixmap);
	 painter.setRenderHint(QPainter::Antialiasing);
	 plot.render(&painter);
	 painter.end();
	 
	 pixmap.save("/Users/mboots/scene.png");
	 */

	 
	return app.exec();
 }
