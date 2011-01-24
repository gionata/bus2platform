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

bool IntervalPackingFirstFit::solveX()
{
	_clock_begin = clock();

	Intervals &I = _sets.I();
	int dwellsToBeAssigned = I.size();
	int numDwells = dwellsToBeAssigned;
	int gatesUsage = 0;
	int last_color = 0;
	ptime w[_sets.G().size()]; // _graphs.sets().lowerBoundNumberGates()
	bool used[_sets.G().size()];

	for (int i = 0; i < _sets.G().size(); i++) {
		w[i] = _sets.G().at(i)->stopAvailableTimes().front()->begin();
		used[i] = false;
	}
	/* Ordina I per tempo vincoli più stringenti e per tempo di inizio intervallo */
	sort(I.begin(), I.end(), LeftEndpoint());
	/* Inserisci l'intervallo nel gate aperto con tempo di fine intervallo più vicino */

	for (Intervals::const_iterator v_itr = I.begin();
	        v_itr != I.end(); v_itr++) {
		VertexDescriptorH v_i =
		    vertex((*v_itr)->vertex(), _graphs.graphH());
		// std::cout << "Provo ad assegnare " << (char) ((*v_itr)->vertex() + 'A') << " (arrival = " << (*v_itr)->lower().time_point() << ")" << std::endl;
		for (Gates::const_iterator c_itr = _sets.G().begin();
		        c_itr != _sets.G().end(); c_itr++) {

			VertexDescriptorH v_k =
			    vertex((*c_itr)->id(), _graphs.graphH());

			std::pair < EdgeDescriptorH, bool > edge_ik =
			    edge(v_i, v_k, _graphs.graphH());


			// std::cout << "  provo la piattaforma " << ((*c_itr)->id() - numDwells) << " (fine occupazione = " << w[(*c_itr)->id() - numDwells] << ")" << std::endl;
			// std::cout << "    testo " << (*v_itr)->lower().time_point() << " < " << w[(*c_itr)->id() - numDwells] << std::endl;
			if (edge_ik.second == false) {// non assegnabile
				//std::cout << "    non sono compatibili" << std::endl;
				continue;
			} else if ((*v_itr)->lower().time_point() < w[(*c_itr)->id() - numDwells]) { // piattaforma occupata
				// std::cout << "    piattaforma occupata " << (*v_itr)->lower().time_point() << " " << w[(*c_itr)->id() - numDwells] << std::endl;
				continue;
			} else { // assegnabile
				dwellsToBeAssigned--;
				if ((*c_itr)->id() - numDwells > last_color)
					last_color = (*c_itr)->id() - numDwells;
				(*v_itr)->platform((*c_itr)->id());
				w[(*c_itr)->id() - numDwells] = (*v_itr)->upper().time_point();
				if (!used[(*c_itr)->id() - numDwells]) {
					used[(*c_itr)->id() - numDwells] = true;
					gatesUsage++;
					// DEBUG
					// std::cout << "  uso " << (*c_itr)->id() - numDwells + 1 << std::endl;

				}
				// DEBUG
				//std::cout << "      assegnato " << _sets.B()[(*v_itr)->vertex()]->dwellNumber() << " a " << (*c_itr)->id() - numDwells + 1 << " [" << numDwells - dwellsToBeAssigned << "]" << std::endl;
				break;
			}
		}
	}
	_clock_end = clock();
	_objectiveFunction = gatesUsage;
	_solved = ! dwellsToBeAssigned;
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
