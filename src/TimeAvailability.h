/*
 * PlatformAvailability.h
 *
 *  Created on: 14/gen/2010
 *      Author: Gionata
 */

#ifndef MODEL_TIMEAVAILABILITY_H_
#define MODEL_TIMEAVAILABILITY_H_

#include "CalendarDate.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;

class TimeAvailability {
public:
	TimeAvailability(CalendarDate *day, ptime begin, ptime end);
	~TimeAvailability();
	CalendarDate *day() const;
	ptime begin() const;
	ptime end() const;
	time_duration time_slot() const;
private:
	CalendarDate *_day;
	ptime _begin;
	ptime _end;
	time_duration _time_slot;
};

#endif /* MODEL_TIMEAVAILABILITY_H_ */
