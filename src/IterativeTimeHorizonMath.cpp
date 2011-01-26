/*! \file IterativeTimeHorizonMath.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "IterativeTimeHorizonMath.h"

using namespace std;
using namespace boost;

IterativeTimeHorizonMath::IterativeTimeHorizonMath(GraphModel &graphs):
	_graphs(graphs),
	_H(graphs.graphH()),
	_C(graphs.graphC())
{
	_assignment = new std::vector < std::pair < int, int > >(_graphs.numEdgesH());
	findTotalHorizon();
}

IterativeTimeHorizonMath::~IterativeTimeHorizonMath()
{
	delete _assignment;
}

bool IterativeTimeHorizonMath::solution(int *&gates) const
{
  return true;
}

bool IterativeTimeHorizonMath::initialSolution(int *startingSolution)
{

}

bool IterativeTimeHorizonMath::solveX()
{
	cout << "Pianifico in [" << _opening << ", " << _closing << "]" << endl;
	return true;
}

void
IterativeTimeHorizonMath::findTotalHorizon()
{
	_opening = ptime (date(1980,Jan,21), time_duration(7,10,0));
	_closing = ptime (date(2020,Jan,21), time_duration(7,10,0));
	
	for (Gates::const_iterator giter = _graphs.G().begin(); giter != _graphs.G().end();
	        giter++) {
		std::vector < TimeAvailability * >&availableTimes =
		    (*giter)->stopAvailableTimes();

		for (std::vector <
		        TimeAvailability * >::const_iterator availableTimeIter =
		            availableTimes.begin();
		        availableTimeIter != availableTimes.end();
		        availableTimeIter++) {
			ptime beginTime = (*availableTimeIter)->begin();
			ptime endTime = (*availableTimeIter)->end();

			if (beginTime < _opening) {
				_opening = beginTime;
			}

			if (endTime > _closing) {
				_closing = endTime;
			}
		}
	}
}