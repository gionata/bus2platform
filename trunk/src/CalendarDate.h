/*
 * CalendarDate.h
 *
 *  Created on: 13/gen/2010
 *      Author: Gionata
 */

#ifndef MODEL_CALENDARDATE_H_
#define MODEL_CALENDARDATE_H_

#include "Calendar.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;

class CalendarDate {
public:
	CalendarDate();
	virtual ~ CalendarDate();
private:
	/** A set of dates when a service exception is available for one or more routes. */
	Calendar *calendar;
	/** The date field specifies a particular date when service availability is different than the norm. You can use the exception_type  field to indicate whether service is available on the specified date. */
	date day;
	/** The exception_type indicates whether service is available on the date specified in the date  field.
	 *    - A value of 1 indicates that service has been added for the specified date.
	 *   - A value of 2 indicates that service has been removed for the specified date.
	 */
	int exception_type;
};

#endif /* MODEL_CALENDARDATE_H_ */
