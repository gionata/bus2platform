#ifndef GANTTDIAGRAM_H
#define GANTTDIAGRAM_H

#include "Gate.h"
#include "Bus.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <cairo/cairo-svg.h>
#include <cairo/cairo-pdf.h>

#ifdef _B
#	undef _B
#endif

enum surface_t {pdf = 0, svg};

class GanttDiagram {
public:
	GanttDiagram(const char *output, Gates &G, Buses &B, int *dwellOnPlatform,
           	unsigned int used_platform,
           	unsigned int min_interval_distance,
           	double mean_interval,
		double opt_time,
           	surface_t st = pdf);
	~GanttDiagram(void);
private:
	Gates & _G;
	Buses & _B;
	int *_dwellOnPlatform;
	double _width;
	double _x_offset;
	double _y_offset;
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
	void drawBorderedRectangle(double x0, double y0, double width, double height, double rBorder = 0.0, double gBorder = 0.0, double bBorder = 0.0, double rFill = 1.0, double gFill = 1.0, double bFill = 1.0, cairo_pattern_t *hatch = 0);
	boost::posix_time::ptime _minTime, _maxTime;
	boost::posix_time::time_duration _diffTime;

	cairo_surface_t *_surface;
	cairo_t *_cr;
};

#endif /* GANTTDIAGRAM_H */
