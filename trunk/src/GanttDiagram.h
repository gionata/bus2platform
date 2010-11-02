#ifndef GANTTDIAGRAM_H
#define GANTTDIAGRAM_H

#include "Gate.h"
#include "Bus.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <cairo/cairo-svg.h>

#ifdef _B
#	undef _B
#endif

class GanttDiagram {
public:
	GanttDiagram(const char *output, Gates &G, Buses &B, int *dwellOnPlatform);
	~GanttDiagram(void);
private:
	Gates & _G;
	Buses & _B;
	int *_dwellOnPlatform;
	double _width;
	double _x_offset;
	double _height;
	double _platformHeight;
	double *_platform_y_offset;
	int _platforms;
	double _scale;
	double findHeight();
	double findWidth();
	double x_coordinate(boost::posix_time::ptime p);
	void drawPlatforms();
	void drawBusDwells();
	void drawBorderedRectangle(double x0, double y0, double width, double height, double rBorder = 0.0, double gBorder = 0.0, double bBorder = 0.0, double rFill = 1.0, double gFill = 1.0, double bFill = 1.0);

	cairo_surface_t *_surface;
	cairo_t *_cr;
};

#endif /* GANTTDIAGRAM_H */
