/*! \file IntervalEndpoint.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "IntervalEndpoint.h"

IntervalEndpoint::IntervalEndpoint(ptime time_point, EndPointType endPoint,
                 int vertex): _time_point(time_point),
    _endPoint(endPoint), _vertex(vertex) {
}

IntervalEndpoint::IntervalEndpoint() {
}

IntervalEndpoint::~IntervalEndpoint()
{
    // TODO Auto-generated destructor stub
}

bool IntervalEndpoint::operator<(const IntervalEndpoint &right) {
    return _time_point < right._time_point;
}

bool IntervalEndpoint::cmp(const IntervalEndpoint &left,
                                  const IntervalEndpoint &right) {
    return left._time_point < right._time_point;
}

EndPointType IntervalEndpoint::endpointType() const {
    return _endPoint;
}

int IntervalEndpoint::vertex() const {
    return _vertex;
}

ptime IntervalEndpoint::time_point() const {
    return _time_point;
}
