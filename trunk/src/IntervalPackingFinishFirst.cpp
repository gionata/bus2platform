/*! \file IntervalPackingFinishFirst.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "IntervalPackingFinishFirst.h"
#include "gap.h"
#include <list>
#include <algorithm>

using namespace std;

IntervalPackingFinishFirst::IntervalPackingFinishFirst(SetModel &sets, GraphModel &graphs): _sets(sets),
	_graphs
	(graphs)
{
	_solved = false;

	Intervals &I = _sets.I();
	for (Intervals::const_iterator iitr = I.begin();
	        iitr != I.end(); iitr++) {
		(*iitr)->assigned(false);
	}
}

IntervalPackingFinishFirst::~IntervalPackingFinishFirst()
{
	// TODO Auto-generated destructor stub
}

bool IntervalPackingFinishFirst::solution(int *&gates) const
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

	//////////////   VERIFICA AMMISSIBILITA   //////////////
	//                                                    //
	//     verifica di ammissibilita' della soluzione     //
	//                                                    //
	////////////////////////////////////////////////////////
	vector < vector <size_t> > gate_dwell (_objectiveFunction);
	bool used[(int)_objectiveFunction];
	for (int i = 0; i < _objectiveFunction; i++) {
		used[i] = false;
	}
	// setta il vettore gate_dwell
	for (Intervals::const_iterator dwellIntervall = I.begin();
	        dwellIntervall != I.end(); dwellIntervall++) {
		size_t platform = (*dwellIntervall)->platform() - numDwells;
		if (!used[platform]) {
			used[platform] = true;
			// gate_dwell[platform]= new vector<size_t>();
		}
		gate_dwell[platform].push_back((*dwellIntervall)->vertex());
	}
	// controlla, per ogni piattaforma
	for (int p = 0; p < _objectiveFunction; p++) {
		// che ogni sosta
		for (vector<size_t>::const_iterator i = gate_dwell[p].begin(); i != gate_dwell[p].end(); i++) {
			// sia assegnabile alla piattaforma
			if (! _graphs.B()[*i]->compatible(_graphs.G()[p])) {
				cerr << "sosta " << _graphs.B()[*i]->dwellNumber() << " non compatibile con piattaforma "<< p + 1 << endl;
				exit(1);
			}
			// non interferisca con nessuna delle soste successive
			if (*i != gate_dwell[p].back()) {
				vector<size_t>::const_iterator j = i;
				j++;
				for (; j != gate_dwell[p].end(); j++) {
					if (_graphs.B()[*i]->occupacyPeriod().intersects(_graphs.B()[*j]->occupacyPeriod())) {
						cerr << "piattaforma " << p + 1 << ", sosta " <<
						     _graphs.B()[*i]->dwellNumber() << " interferisce con " << _graphs.B()[*j]->dwellNumber() << endl;
						exit(1);
					}
				}
			}
		}
	}

	return true;
}

bool IntervalPackingFinishFirst::solveX()
{
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

	_clock_begin = clock();

	Intervals &I = _sets.I();
	int dwellsToBeAssigned = I.size();
	int gatesUsage = 0;
	/* Ordina I per tempo di fine intervallo */
	sort(I.begin(), I.end(), RightEndpoint());

	for (Gates::const_iterator gitr = _sets.G().begin();
	        dwellsToBeAssigned && gitr != _sets.G().end(); gitr++) {
		list < Interval * >S;
		list < Interval * >Ik;

		for (Intervals::const_iterator iitr = I.begin();
		        iitr != I.end(); iitr++) {
			if (!(*iitr)->assigned()) {
				boost::graph_traits <
				GraphH >::vertex_descriptor v_i =
				    vertex((*iitr)->vertex(), _graphs.graphH());
				boost::graph_traits <
				GraphH >::vertex_descriptor v_k =
				    vertex((*gitr)->id(), _graphs.graphH());
				std::pair < boost::graph_traits <
				GraphH >::edge_descriptor, bool > edge_ik =
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
				if ((*otherInterval)->overlap(*finish_first)
				        /*	|| ( (*otherInterval)->lower().time_point() > finish_first->upper().time_point()
				        		&& (*otherInterval)->lower().time_point() - minutes(10) < finish_first->upper().time_point())
				        	|| ( (*otherInterval)->upper().time_point() > finish_first->lower().time_point()
				        		&& (*otherInterval)->upper().time_point() - minutes(10) < finish_first->lower().time_point())
				        */		) {
					Interval *overlappingInterval =
					    *otherInterval;
					otherInterval++;
					Ik.remove(overlappingInterval);
				} else {
					otherInterval++;
				}
			}
		}

		/*
		   cout << "Gate " << (*gitr)->gateNumber() << " (" << S.size() << ")[";
		   for (list<Interval *>::const_iterator currentGateInterval = S.begin(); currentGateInterval != S.end(); currentGateInterval++) {
		   cout << " " << (*currentGateInterval)->vertex();
		   }
		   cout << " ]" << endl;
		 */
	}

	// cerr << "Usati " << gatesUsage << " gates." << endl;

	if (dwellsToBeAssigned <= 0) {
		_solved = true;
	}

	_objectiveFunction = gatesUsage;

	_clock_end = clock();

	return _solved;
}
