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
	Intervals &I = _sets.I();
	int dwellsToBeAssigned = I.size();
	int gatesUsage = 0;
	/* Ordina I per tempo vincoli più stringenti e per tempo di inizio intervallo */
	sort(I.begin(), I.end(), LeftEndpoint());
	/* Inserisci l'intervallo nel gate aperto con tempo di fine intervallo più vicino */

	for (Gates::const_iterator gitr = _sets.G().begin();
			dwellsToBeAssigned && gitr != _sets.G().end(); gitr++) {
		list < Interval * >S;
		list < Interval * >Ik;

		for (Intervals::const_iterator iitr = I.begin();
				iitr != I.end(); iitr++) {
			if (!(*iitr)->assigned()) {
				VertexDescriptorH v_i =
					vertex((*iitr)->vertex(), _graphs.graphH());
				VertexDescriptorH v_k =
					vertex((*gitr)->id(), _graphs.graphH());
				std::pair < EdgeDescriptorH, bool > edge_ik =
					edge(v_i, v_k, _graphs.graphH());

				if (edge_ik.second == false) {
					continue;
				}

				Ik.push_back(*iitr);
			}
		}

		gatesUsage++;

		while (!Ik.empty()) {
			Interval *finish_first = Ik.front();
			S.push_back(finish_first);
			Ik.remove(finish_first);
			finish_first->platform((*gitr)->id());
			dwellsToBeAssigned--;
			list < Interval * >::iterator otherInterval =
				Ik.begin();

			while (otherInterval != Ik.end()) {
				if ((*otherInterval)->overlap(*finish_first)) {
					Interval *overlappingInterval =
						*otherInterval;
					otherInterval++;
					Ik.remove(overlappingInterval);
				} else {
					otherInterval++;
				}
			}
		}

		cout << "Gate " << (*gitr)->gateNumber() << " (" << S.
			size() << ")[";

		for (list < Interval * >::const_iterator currentGateInterval =
					S.begin(); currentGateInterval != S.end();
				currentGateInterval++) {
			cout << " " << (*currentGateInterval)->vertex();
		}

		cout << " ]" << endl;
	}

	cout << "Usati " << gatesUsage << " gates." << endl;
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
