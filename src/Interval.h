/*! \file Interval.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef INTERVAL_H_
#define INTERVAL_H_

#include "IntervalEndpoint.h"

#include <vector>

/**
 *
 */
class Interval {
public:
	Interval(IntervalEndpoint const &l, IntervalEndpoint const &u, int platform = -1, bool assigned = false);
	// Interval();
	~Interval();
	IntervalEndpoint const &lower() const;
	IntervalEndpoint const &upper() const;
	bool assigned() const;
	void assigned(bool);
	int platform() const;
	void platform(int g);
	bool overlap(const Interval &x);
	int vertex() const;
private:
	const IntervalEndpoint &_low;
	const IntervalEndpoint &_up;
	bool _assigned;
	int _platform;
};

typedef std::vector < Interval * >Intervals;

class RightEndpoint {
public:
	bool operator() (const Interval *i1, const Interval *i2) const {
		return i1->upper().time_point() < i2->upper().time_point();
	}
};

class LeftEndpoint {
public:
	bool operator() (const Interval *i1, const Interval *i2) const {
		return i1->lower().time_point() < i2->lower().time_point();
	}
};

class LeftRightEndpoint {
public:
	bool operator() (const Interval *i1, const Interval *i2) const {
		if (i1->lower().time_point() < i2->lower().time_point())
			return true;
		else if (i1->lower().time_point() <= i2->lower().time_point())
			return i1->upper().time_point() <= i2->upper().time_point();
		else
			return false;
	}
};

#endif /* INTERVAL_H_ */
