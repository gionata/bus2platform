#include "GanttDiagram.h"
#include "TimeSlots.h"

#include <cstdio>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;

GanttDiagram::GanttDiagram(const char *output, Gates &G, Buses &B,
                           int *dwellOnPlatform,
                       	unsigned int used_platform,
                       	unsigned int min_interval_distance,
                       	double mean_interval,
                           surface_t st): _G(G), _B(B),
	_dwellOnPlatform(dwellOnPlatform), _x_offset(0)
{
	_platforms = _G.size();
	_scale = 1.0;
	_width = findWidth();
	_scale = 600 / _width;
	_width = 600;
	_height = 3 * _width / 4;
	//_platformHeight = _height / (2 * _platforms - 1);
	_platformHeight = 2 *_height / (3 * _platforms - 1);
	_y_offset = 1;
	_platform_y_offset = new double[_platforms];
	_platform_y_offset[0] = _y_offset;

	//std::cout << "width x height = " << _width << " x " << _height << std::endl;
	//std::cout << "platform height = " << _platformHeight << std::endl;

	for (int p = 1; p < _platforms; p++) {
		// _platform_y_offset[p] = 2 * p * _platformHeight;
		_platform_y_offset[p] = 3.0 / 2 * p * _platformHeight + _y_offset;
		//std::cout << "platform offset[" << p << "] = " << _platform_y_offset[p] << std::endl;
	}

	if (st == svg)
		_surface = cairo_svg_surface_create(output, _width * 4.0 / 3, _height * 4.0 / 3);
	else if (st == pdf)
		_surface = cairo_pdf_surface_create(output, _width * 4.0 / 3, _height * 4.0 / 3);
	_cr = cairo_create(_surface);
	cairo_select_font_face(_cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
	                       CAIRO_FONT_WEIGHT_NORMAL);
	// cairo_set_font_size(_cr, _platformHeight / 3); // 800pt
	cairo_set_font_size(_cr, _platformHeight / 6); // 600pt

	drawPlatforms();
	drawBusDwells();


	cairo_text_extents_t extents;
	char text[50];
	sprintf(text, "Piattaforme: %d", used_platform);
	cairo_text_extents(_cr, text, &extents);
	cairo_move_to(_cr, _x_offset, _height * 7.0 / 6);
	cairo_set_source_rgb(_cr, 0, 0, 0);
	cairo_show_text(_cr, text);

	sprintf(text, "Minimo intervallo: %d", (int)min_interval_distance);
	cairo_text_extents(_cr, text, &extents);
	cairo_move_to(_cr, _x_offset, _height * 7.0 / 6 + 20);
	cairo_set_source_rgb(_cr, 0, 0, 0);
	cairo_show_text(_cr, text);

	sprintf(text, "Media intervalli: %g", mean_interval);
	cairo_text_extents(_cr, text, &extents);
	cairo_move_to(_cr, _x_offset, _height * 7.0 / 6 + 40);
	cairo_set_source_rgb(_cr, 0, 0, 0);
	cairo_show_text(_cr, text);

	cairo_surface_destroy(_surface);
	cairo_destroy(_cr);
}

GanttDiagram::~GanttDiagram(void)
{
	delete[]_platform_y_offset;
}

double
GanttDiagram::findWidth()
{
	double min = INT_MAX;
	double max = 0;
	double tmp;

	for (Gates::const_iterator giter = _G.begin(); giter != _G.end();
	        giter++) {
		std::vector < TimeAvailability * >&availableTimes =
		    (*giter)->stopAvailableTimes();

		for (std::vector <
		        TimeAvailability * >::const_iterator availableTimeIter =
		            availableTimes.begin();
		        availableTimeIter != availableTimes.end();
		        availableTimeIter++) {
			ptime beginTime = (*availableTimeIter)->begin();
			ptime endTime = (*availableTimeIter)->end();
			tmp = x_coordinate(beginTime);

			if (tmp < min) {
				min = tmp;
			}

			tmp = x_coordinate(endTime);

			if (tmp > max) {
				max = tmp;
			}
		}
	}

	_x_offset = min - 1;

	return max - min;
}

void GanttDiagram::drawPlatforms()
{
	cairo_text_extents_t extents;
	for (int p = 0; p < _platforms; p++) {
		// per ogni intervallo di abilitazione disegna il rettangolo corrispondente
		std::vector < TimeAvailability * >&stopAvailableTimes =
		    _G[p]->stopAvailableTimes();

		for (std::vector <
		        TimeAvailability * >::const_iterator interval =
		            stopAvailableTimes.begin();
		        interval != stopAvailableTimes.end(); interval++) {
			double x0 = x_coordinate((*interval)->begin());
			double x1 = x_coordinate((*interval)->end());

			drawBorderedRectangle(x0, _platform_y_offset[p], x1 - x0, _platformHeight
			                      , 0, 0, 0.5
			                      , 0.7, 0.7, 0.7);
			char platform_number[5];
			sprintf(platform_number, "P. %d", _G[p]->gateNumber());
			cairo_text_extents(_cr, platform_number, &extents);
			cairo_move_to(_cr, _width * 7.0 / 6,
		              _platform_y_offset[p] + _platformHeight / 2);
			cairo_set_source_rgb(_cr, 0, 0, 0);
			cairo_show_text(_cr, platform_number);
		}
	}
}

void GanttDiagram::drawBusDwells()
{
	cairo_text_extents_t extents;

	for (Buses::const_iterator b = _B.begin(); b != _B.end(); b++) {
		int bus_index = (*b)->id();
		int gate = _dwellOnPlatform[bus_index];
		ptime arrival = (*b)->arrival();
		ptime departure = (*b)->departure();
		double x0 = x_coordinate(arrival);
		double x1 =  x_coordinate(departure);

		drawBorderedRectangle(x0, _platform_y_offset[gate], x1 - x0, _platformHeight
		                      , 0.5, 0, 0
		                      , 1.0, 0.97, 0.80);
		char bus_number[5];
		sprintf(bus_number, "%d", (*b)->dwellNumber());
		cairo_text_extents(_cr, bus_number, &extents);
		cairo_move_to(_cr, (x1 + x0) / 2 - extents.width / 2,
		              _platform_y_offset[gate] + _platformHeight / 2);
		cairo_set_source_rgb(_cr, 0, 0, 0);
		cairo_show_text(_cr, bus_number);
	}
}

double GanttDiagram::x_coordinate(ptime p)
{
	long ss = p.time_of_day().seconds();
	long mm = p.time_of_day().minutes();
	long hh = p.time_of_day().hours();
	double x = (1. / 60 * ss +
	            1. * mm +
	            60. * hh -
	            _x_offset) * _scale;

	return x;
}

void GanttDiagram::drawBorderedRectangle(double x0, double y0, double width, double height, double rBorder, double gBorder, double bBorder, double rFill, double gFill, double bFill)
{
	cairo_rectangle(_cr, x0, y0, width, height);
	cairo_set_source_rgb(_cr, rFill, gFill, bFill);
	cairo_fill_preserve(_cr);
	cairo_set_source_rgb(_cr, rBorder, gBorder, bBorder);
	cairo_set_line_width(_cr, _platformHeight / 50);
	cairo_stroke(_cr);
}
