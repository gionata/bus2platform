#ifndef TIMESLOTS_H
#define TIMESLOTS_H

#include "TimeAvailability.h"
#include "CalendarDate.h"

#include <vector>
#include <algorithm>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;

class TimeSlots {
public:
	static TimeSlots &getInstance();
	//TimeSlots &getInstance();
	TimeAvailability *getTimeAvailability(CalendarDate *day, ptime begin,
	                                      ptime end);
private:
	TimeSlots(void);
	~TimeSlots(void);
	TimeSlots(const TimeSlots &);
	TimeSlots &operator=(const TimeSlots &);
	std::vector < TimeAvailability * >_times;
	static TimeSlots *_timeSlots;
};

#endif /* TIMESLOTS_H */
