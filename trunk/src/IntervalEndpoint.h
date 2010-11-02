/*! \file IntervalEndpoint.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef INTERVALENDPOINT_H_
#define INTERVALENDPOINT_H_

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

typedef enum EndPointType_t { rightEP = 0, leftEP = 1 } EndPointType;

/**
 *
 */
class IntervalEndpoint {
public:
    IntervalEndpoint();
    IntervalEndpoint(ptime time_point, EndPointType endPoint,
                     int vertex);
    ~IntervalEndpoint();
    bool operator<(const IntervalEndpoint &right);
    static bool cmp(const IntervalEndpoint &left,
                    const IntervalEndpoint &right);
    EndPointType endpointType() const;
    int vertex() const;
    ptime time_point() const;
private:
    ptime _time_point;
    EndPointType _endPoint;
    int _vertex;
};

class EndpointLess {
public:
    bool operator() (const IntervalEndpoint &left,
                     const IntervalEndpoint &right) const {
        return left.time_point() < right.time_point();
    }
};

#endif /* INTERVALENDPOINT_H_ */
