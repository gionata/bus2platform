/*! \file IntervalPackingFirstFit.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "IntervalPackingFirstFit.h"
#include "gap.h"
#include <list>
#include <algorithm>

using namespace std;

IntervalPackingFirstFit::IntervalPackingFirstFit(SetModel &sets, GraphModel &graphs): _sets(sets),
     _graphs
     (graphs)
{
}

/*
Given a set of overlapping intervals, find a largest subset of pairwise-disjoint intervals.
Given a set of n intervals I = {i_1, i_2, ...,i_n}, where the k^th interval starts at time s_k
and finishes at time f_k, find a largest subset of intervals that do not overlap.

for Gate k
  S[k] <- 0;
  I[k] <- i | i compatible with k and free(i)
  while I[k] != 0;
    remove from I[k] the interval i with the smallest f
    insert i into S[k]
    remove from Ik all intervals that overlap with i
return S
*/

IntervalPackingFirstFit::~IntervalPackingFirstFit()
{
     // TODO Auto-generated destructor stub
}

bool IntervalPackingFirstFit::solveX() {
     _clock_begin = clock();

     Intervals &I = _sets.I();
     int dwellsToBeAssigned = I.size();
     int gatesUsage = 0;
     int last_color = 0;
     ptime w[_sets.G().size()]; // _graphs.sets().lowerBoundNumberGates()

     for (int i = 0; i < _sets.G().size(); i++) {
          w[i] = _sets.G().at(i)->stopAvailableTimes().front()->begin();
     }
     /* Ordina I per tempo vincoli più stringenti e per tempo di inizio intervallo */
     sort(I.begin(), I.end(), LeftEndpoint());
     /* Inserisci l'intervallo nel gate aperto con tempo di fine intervallo più vicino */

     for (Intervals::const_iterator v_itr = I.begin();
               v_itr != I.end(); v_itr++) {
          VertexDescriptorH v_i =
               vertex((*v_itr)->vertex(), _graphs.graphH());
          for (Gates::const_iterator c_itr = _sets.G().begin();
                    c_itr != _sets.G().end(); c_itr++) {

               VertexDescriptorH v_k =
                    vertex((*c_itr)->id(), _graphs.graphH());

               std::pair < EdgeDescriptorH, bool > edge_ik =
                    edge(v_i, v_k, _graphs.graphH());

               if (edge_ik.second == false // non assegnabile
                         || (*v_itr)->lower().time_point() < w[(*c_itr)->id()]) { // piattaforma occupata
                    continue;
               } else { // assegnabile
                    dwellsToBeAssigned--;
                    if ((*c_itr)->id()>last_color)
                         last_color = (*c_itr)->id();
                    (*v_itr)->platform((*c_itr)->id());
                    w[(*c_itr)->id()] = (*v_itr)->upper().time_point();
               }
          }
     }
}

bool IntervalPackingFirstFit::solution(int*& gates) const
{
	if (!_solved) {
		return false;
	}

	Intervals &I = _sets.I();
	int numDwells = _sets.B().size();
	gates = new int[numDwells];

	for (Intervals::const_iterator dwellIntervall = I.begin();
			dwellIntervall != I.end(); dwellIntervall++) {
		gates[(*dwellIntervall)->vertex()] =
			(*dwellIntervall)->platform() - numDwells;
	}

	return true;
}
