/*
 * PlatformUnavailability.cpp
 *
 *  Created on: 14/gen/2010
 *      Author: Gionata
 */

#include "TimeAvailability.h"

TimeAvailability::TimeAvailability(CalendarDate *day, ptime begin, ptime end):
    _day(day), _begin(begin), _end(end)
{
    _time_slot = end - begin;
}

TimeAvailability::~TimeAvailability()
{
    // TODO Auto-generated destructor stub
}

CalendarDate *TimeAvailability::day() const
{
    return _day;
}

ptime TimeAvailability::begin() const
{
    return _begin;
}

ptime TimeAvailability::end() const
{
    return _end;
}

time_duration TimeAvailability::time_slot() const
{
    return _time_slot;
}
