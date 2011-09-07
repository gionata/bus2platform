/*! \file IterativeTimeHorizonMath.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "IterativeTimeHorizonMathMD.h"

using namespace std;
using namespace boost;

IterativeTimeHorizonMathMD::IterativeTimeHorizonMathMD(GraphModel &graphs):
	_graphs(graphs),
	_H(graphs.graphH()),
	_C(graphs.graphC())
{
	_assignment = new std::vector < std::pair < int, int > >(_graphs.numEdgesH());
	findTotalHorizon();
}


IterativeTimeHorizonMathMD::~IterativeTimeHorizonMathMD()
{
	delete _assignment;
}

bool IterativeTimeHorizonMathMD::solution(int *&gates) const
{
	gates = new int[_graphs.B().size()];
	for (int i = 0; i < _graphs.B().size(); i++)
		gates[i] = _graphs.B()[i]->platform();
	// delete[] gates;

	for (int i = 0; i < _graphs.B().size(); i++) {
			_graphs.B()[i]->assigned(false);
	}

	return true;
}

bool IterativeTimeHorizonMathMD::initialSolution(int *startingSolution)
{

}

bool IterativeTimeHorizonMathMD::solveX()
{
	int *solution = 0;
	bool controlPrevious = false;
	ptime p_prev = _opening;
	ptime p_curr;
	_clock_begin = clock();
	int interval = 0;
	/* Per ogni ogni intervallo di pianificazione */
	for (p_curr = _opening; p_curr < _closing; p_prev = p_curr, p_curr += minutes(30), interval++) {
		ptime end = p_curr + minutes(60);
		// cout << "   Pianifico in [" << p_curr << ", " << end << "] considerando [" << p_prev << ", " << end << "]" << endl;

		/* genera il nuovo insieme delle soste */
		Buses dwellsWithinPeriod;
		Gates platformsWithinPeriod;
		int id = 0;
		for (Gates::const_iterator gitr = _graphs.G().begin(); gitr != _graphs.G().end(); gitr++) {
			Gate *g = new Gate(*(*gitr));
			g->id(id++);
			platformsWithinPeriod.push_back(g);
		}
		vector <int> mapVectorId;
		id = 0;
		for (Buses::const_iterator bitr = _graphs.B().begin(); bitr != _graphs.B().end(); bitr++) {
			if ( ((*bitr)->departure() >= p_prev) && ((*bitr)->arrival() <= end)) {
				// rimappa i gates
				Gates *gates = new Gates();
				for (Gates::const_iterator gitr = (*bitr)->gates().begin(); gitr != (*bitr)->gates().end(); gitr++) {
					int gateVectorIndex = (*gitr)->id() - _graphs.B().size();
					gates->push_back(platformsWithinPeriod[gateVectorIndex]);
				}
				Bus *dwell = new Bus(*(*bitr), id, gates);
				// aggiungilo alle soste correnti
				dwellsWithinPeriod.push_back(dwell);
				// mappa le corrispondenze
				mapVectorId.push_back((*bitr)->id());
				id++;
			}
		}

		if (dwellsWithinPeriod.size() == 0) {
			controlPrevious = true;
			continue;
		}

		SetModel pSet(dwellsWithinPeriod, platformsWithinPeriod);
		GraphModel pGraph(pSet);
		MathModel *mMaxMinDistance = new MathModelMaxMinDistance(pGraph);

		/*
		if (controlPrevious) {
			// Forza gli assegnamenti generati all'iterazione precedente
		}
		*/
		
		//cout << "MathModel generato" << endl;
		// Risolvi il modello
		mMaxMinDistance->verbose(IMPORTANT); // NORMAL);
		mMaxMinDistance->setTimeout(30);

		char itrNo[4];
		char fn[40];
		sprintf(itrNo, "%03d", interval); 
		strcpy(fn, "modelIterativeMMD_");
		strcat(fn, itrNo);
		strcat(fn, ".lp");
		mMaxMinDistance->writeModelLP_solve(fn);
		mMaxMinDistance->solveX();

		if (mMaxMinDistance->solved()) {
			// Imposta gli assegnamenti determinati nell'iterazione corrente
			int *pSolution = 0;
			mMaxMinDistance->solution(pSolution);
			for (int i = 0; i < dwellsWithinPeriod.size(); i++) {
				_graphs.B()[mapVectorId[i]]->assigned(true);
				_graphs.B()[mapVectorId[i]]->platform(pSolution[i]);
			}
		} else
			return false;
	}

	_clock_end = clock();
	
	return true;
}

void
IterativeTimeHorizonMathMD::findTotalHorizon()
{
	_opening = ptime (date(2020,Jan,21), time_duration(7,10,0));
	_closing = ptime (date(1980,Jan,21), time_duration(7,10,0));
	
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
