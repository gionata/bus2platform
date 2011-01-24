/*! \file Gate.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef GATE_H_
#define GATE_H_

/**
 *
 */
#include "has_id.h"
#include "TimeAvailability.h"

#include <vector>

class Gate: public has_id {
public:
	Gate(int id, int gateNumber);
	~Gate();
	int gateNumber() const;
	std::vector < TimeAvailability * > &stopAvailableTimes();
	void stopAvailableTimes(TimeAvailability *timeAvailability);
	size_t vertexIndex() const;
	void vertexIndex(size_t vertexIndex);
private:
	int _gateNumber;
	std::vector < TimeAvailability * > *_stopAvailableTimes;
	size_t _vertexIndex;
};

typedef std::vector < Gate * > Gates;

#endif /* GATE_H_ */
