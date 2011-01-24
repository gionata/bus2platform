/*! \file Interval.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "Interval.h"

//Interval::Interval() {
//}

Interval::~Interval()
{
	// TODO Auto-generated destructor stub
	delete &_low;
	delete &_up;
}

Interval::Interval(IntervalEndpoint const &l, IntervalEndpoint const &u, int platform, bool assigned): _low(l), _up(u), _assigned(assigned),
	_platform(platform)
{
}

IntervalEndpoint const & Interval::lower() const
{
	return _low;
}

IntervalEndpoint const & Interval::upper() const
{
	return _up;
}

bool Interval::assigned() const
{
	return _assigned;
}

void Interval::assigned(bool b)
{
	_assigned = b;
}

int Interval::platform() const
{
	return _platform;
}

void Interval::platform(int g)
{
	_platform = g;
	_assigned = true;
}

/*static bool overlap(const Interval& x, const Interval& y)
   {
   return x.lower().time_point() <= y.lower().time_point() && y.lower().time_point() <= x.upper().time_point() ||
   y.lower().time_point() <= x.lower().time_point() && x.lower().time_point() <= y.upper().time_point();
   } */

bool Interval::overlap(const Interval &x)
{
	return (x.lower().time_point() <= lower().time_point()
	        && lower().time_point() <= x.upper().time_point())
	       || (lower().time_point() <= x.lower().time_point()
	           && x.lower().time_point() <= upper().time_point());
}

int Interval::vertex() const
{
	return _low.vertex();
}
