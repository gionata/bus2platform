/*! \file Bus.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef BUS_H_
#define BUS_H_

/**
 *
 */
#include "has_id.h"
#include "Gate.h"
#include <vector>
#include <algorithm>
#include <cstring>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

class Bus: public has_id {
public:

    Bus(int id, int dwellNumber, ptime arrival, ptime departure,
        Gates *gates = 0);
    ~Bus();
    const ptime arrival() const;
    const ptime departure() const;
    const time_period occupacyPeriod() const;
    int dwellNumber() const;
    const Gates &gates();
    bool compatible(Gate *g) const;
    size_t vertexIndex() const;
    void vertexIndex(size_t vertexIndex);
    bool assigned() const;
    void assigned(bool status);
    size_t platform() const;
    void platform(size_t gate);
private:
    int _dwellNumber;
    ptime _arrival;
    ptime _departure;
    time_period _occupacyPeriod;
    Gates *_gates;
    size_t _vertexIndex;
    bool _assigned;
    size_t _platform;
};

typedef std::vector < Bus * >Buses;

class arrivalTime {
public:
    bool operator() (const Bus *i1, const Bus *i2) const {
        return i1->arrival() < i2->arrival();
    }
};

#endif /* BUS_H_ */
