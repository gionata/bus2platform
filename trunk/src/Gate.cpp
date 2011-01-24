/*! \file Gate.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "Gate.h"

Gate::Gate(int id, int gateNumber): has_id(id), _gateNumber(gateNumber)
{
	_stopAvailableTimes = new std::vector < TimeAvailability * >();
}

Gate::~Gate()
{
	//for (std::vector<TimeAvailability *>::iterator i = _stopAvailableTimes->begin();
	//		i != _stopAvailableTimes->end(); i++)
	//	if (*i)
	//		delete (*i);

	delete _stopAvailableTimes;
}

int Gate::gateNumber() const
{
	return _gateNumber;
}
std::vector < TimeAvailability * > & Gate::stopAvailableTimes()
{
	return *_stopAvailableTimes;
}

void Gate::stopAvailableTimes(TimeAvailability *timeAvailability)
{
	_stopAvailableTimes->push_back(timeAvailability);
}


size_t Gate::vertexIndex() const
{
	return _vertexIndex;
}

void Gate::vertexIndex(size_t vertexIndex)
{
	this->_vertexIndex = vertexIndex;
}
