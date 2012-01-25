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
			double opt_time,
                           surface_t st): _G(G), _B(B),
	_dwellOnPlatform(dwellOnPlatform), _x_offset(0)
{
	_platforms = _G.size();
	_scale = 1.0;
	_width = findWidth();
	_scale = 340 / _width;
	_width = 340;
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
		_surface = cairo_svg_surface_create(output, _width, _height * 4.0 / 3);
	else if (st == pdf)
		_surface = cairo_pdf_surface_create(output, _width, _height * 4.0 / 3);
	_cr = cairo_create(_surface);
	cairo_select_font_face(_cr, "Cairo:Sans", CAIRO_FONT_SLANT_NORMAL,
	                       CAIRO_FONT_WEIGHT_NORMAL);
	// cairo_set_font_size(_cr, _platformHeight / 3); // 800pt
	//cairo_set_font_size(_cr, (int)_platformHeight / 6); // 600pt

	drawPlatforms();
	drawBusDwells();


	cairo_set_font_size(_cr,  10); 
	char text[50];
	sprintf(text, "Numero piattaforme impiegate: %d", used_platform);
	cairo_move_to(_cr, 1, _height * 7.0 / 6 - 10);
	cairo_set_source_rgb(_cr, 0, 0, 0);
	cairo_show_text(_cr, text);

	sprintf(text, "Minimo intervallo: %d (minuti)", (int)min_interval_distance);
	cairo_move_to(_cr, 1, _height * 7.0 / 6 +  5);
	cairo_set_source_rgb(_cr, 0, 0, 0);
	cairo_show_text(_cr, text);

	sprintf(text, "Media intervalli: %g (minuti)", mean_interval);
	cairo_move_to(_cr, 1, _height * 7.0 / 6 + 20);
	cairo_set_source_rgb(_cr, 0, 0, 0);
	cairo_show_text(_cr, text);

	sprintf(text, "Tempo di calcolo: %gms", opt_time);
	cairo_move_to(_cr, 1, _height * 7.0 / 6 + 35);
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

	_x_offset = min;
	
	return max - min;
}

void GanttDiagram::drawPlatforms()
{
	cairo_text_extents_t extents;
	cairo_pattern_t * hatch = 0;
	/***************/
	if (_B.size() < 25)  {
		cairo_t * cr;
		cairo_surface_t *surface;
		surface = cairo_surface_create_similar(cairo_get_target(_cr),CAIRO_CONTENT_COLOR,10,10);
		cr = cairo_create(surface);
		cairo_set_source_rgb(cr, 1,1,1);
		double sq_side = _platformHeight / 4;
		cairo_rectangle(cr, 0, 0, sq_side, sq_side);
		cairo_fill_preserve(cr);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0,0,0);
		
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
		double wd = sq_side / 5;
		cairo_set_line_width(cr, wd);
		cairo_set_line_join(cr,CAIRO_LINE_JOIN_MITER);

		/*
		//case wxBRUSHSTYLE_CROSS_HATCH:
			cairo_move_to(cr, 5, 0);
			cairo_line_to(cr, 5, 10);
			cairo_move_to(cr, 0, 5);
			cairo_line_to(cr, 10, 5);
		*/
		//    case wxBRUSHSTYLE_BDIAGONAL_HATCH:
			cairo_move_to(cr, 0, 10);
			cairo_line_to(cr, 10, 0);
		 /*
		 //   case wxBRUSHSTYLE_FDIAGONAL_HATCH:
			cairo_move_to(cr, 0, 0);
			cairo_line_to(cr, 10, 10);
		 //   case wxBRUSHSTYLE_CROSSDIAG_HATCH:
			cairo_move_to(cr, 0, 0);
			cairo_line_to(cr, 10, 10);
			cairo_move_to(cr, 10, 0);
			cairo_line_to(cr, 0, 10);
		//    case wxBRUSHSTYLE_HORIZONTAL_HATCH:
			cairo_move_to(cr, 0, 5);
			cairo_line_to(cr, 10, 5);
		//    case wxBRUSHSTYLE_VERTICAL_HATCH:
			cairo_move_to(cr, 5, 0);
			cairo_line_to(cr, 5, 10);
		*/

		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_stroke (cr);

		cairo_destroy(cr);
		hatch = cairo_pattern_create_for_surface (surface);
		cairo_surface_destroy(surface);
		cairo_pattern_set_extend (hatch, CAIRO_EXTEND_REPEAT);
	}
		/***************/
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
			                      , 0.75, 0.75, 0.75
					      , hatch);
			
			if (_B.size() < 25) 
			{	
				char platform_number[5];
				sprintf(platform_number, "%d", _G[p]->gateNumber());
				cairo_text_extents(_cr, platform_number, &extents);
				cairo_move_to(_cr, (_width - extents.width) / 2,
				      _platform_y_offset[p] + 5 * _platformHeight / 4 + extents.height/2);
				cairo_set_source_rgb(_cr, 0, 0, 0);
				cairo_show_text(_cr, platform_number);
			}
		}
	}
	cairo_pattern_destroy (hatch);
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
		if (_B.size() < 25) 
		{
			char bus_number[5];
			sprintf(bus_number, "%d", (*b)->dwellNumber());
			cairo_text_extents(_cr, bus_number, &extents);
			cairo_move_to(_cr, (x1 + x0) / 2 - extents.width / 2,
				      _platform_y_offset[gate] + (_platformHeight + extents.height) / 2);
			cairo_set_source_rgb(_cr, 0, 0, 0);
			cairo_show_text(_cr, bus_number);
		}
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

void GanttDiagram::drawBorderedRectangle(double x0, double y0, double width, double height, double rBorder, double gBorder, double bBorder, double rFill, double gFill, double bFill, cairo_pattern_t *hatch)
{	
	cairo_set_source_rgb(_cr, rFill, gFill, bFill);
	cairo_rectangle(_cr, x0, y0, width, height);
	if (hatch)
		cairo_set_source(_cr, hatch);
	cairo_fill_preserve(_cr);
	cairo_set_source_rgb(_cr, rBorder, gBorder, bBorder);
	cairo_set_line_width(_cr, _platformHeight / 20);
	cairo_rectangle(_cr, x0, y0, width, height);
	cairo_stroke(_cr);
}
