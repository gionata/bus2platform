#include "TimeSlots.h"

//
// Static member initialization.
//
TimeSlots *TimeSlots::_timeSlots = 0;

//static
TimeSlots &TimeSlots::getInstance()
{
	if (_timeSlots != 0) {
		return *_timeSlots;
	} else {
		_timeSlots = new TimeSlots;
		return *_timeSlots;
	}
}

TimeSlots::TimeSlots(void)
{
}

TimeSlots::~TimeSlots(void)
{
	if (_timeSlots)
		delete _timeSlots;
}

TimeAvailability *TimeSlots::getTimeAvailability(CalendarDate *day,
		ptime begin, ptime end)
{
	TimeAvailability *ret;

	for (std::vector < TimeAvailability * >::const_iterator t =
				_times.begin(); t != _times.end(); t++) {
		if ((*t)->day() != day) {
			continue;
		}

		if ((*t)->begin() != begin) {
			continue;
		}

		if ((*t)->end() != end) {
			continue;
		}

		return *t;
	}

	ret = new TimeAvailability(day, begin, end);
	_times.push_back(ret);
	return ret;
}
