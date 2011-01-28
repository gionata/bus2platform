/*! \file Bus.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "Bus.h"
#include <boost/graph/graph_concepts.hpp>

Bus::Bus(int id, int dwellNumber, ptime arrival, ptime departure,
         Gates *gates): has_id(id), _dwellNumber(dwellNumber),
	_arrival(arrival), _departure(departure),
	_occupacyPeriod(arrival, departure)
{
	_gates = gates;
}

Bus::Bus(Bus &dwell, int id, Gates *gates):
		has_id(id),
		_arrival(dwell._arrival),
		_departure(dwell._departure),
		_occupacyPeriod(dwell._arrival, dwell._departure),
		_dwellNumber(dwell._dwellNumber), // nome della sosta
		_gates(gates),
		_assigned(dwell._assigned),
		_platform(dwell._platform)
{
}

Bus::~Bus()
{
	if (_gates)
		delete _gates;
}

const ptime Bus::arrival() const
{
	return _arrival;
}

const ptime Bus::departure() const
{
	return _departure;
}

const time_period Bus::occupacyPeriod() const
{
	return _occupacyPeriod;
}

int Bus::dwellNumber() const
{
	return _dwellNumber;
}

const Gates &Bus::gates()
{
	return *_gates;
}

bool Bus::compatible(Gate *g) const
{
	return (find(_gates->begin(), _gates->end(), g) != _gates->end());
}

size_t Bus::vertexIndex() const
{
	return _vertexIndex;
}

void Bus::vertexIndex(size_t vertexIndex)
{
	this->_vertexIndex = vertexIndex;
}

bool Bus::assigned() const
{
	return _assigned;
}

void Bus::assigned(bool status)
{
	_assigned = status;
}

size_t Bus::platform() const
{
	return _platform;
}

void Bus::platform(size_t gate)
{
	_platform = gate;
}
