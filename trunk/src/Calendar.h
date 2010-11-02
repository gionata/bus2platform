#ifndef MODEL_CALENDAR_H_
#define MODEL_CALENDAR_H_

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/date_defs.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;
////using namespace boost::date_time;

class Calendar {
public:
	Calendar();
	virtual ~ Calendar();
private:
	unsigned long int uid;
	/** The service is provided in the week day? Use a day index in boost::date_time::weekdays. */
	bool dayOfWeek[7];
	date startDate;
	date endDate;
};

#endif /* CALENDAR_H_ */
