/*! \file SetModel.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "SetModel.h"
#include "TimeAvailability.h"
#include "TimeSlots.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <string>
#include <cstdio>

SetModel::SetModel(char *filename, int mylag)
{
	int no_platforms;
	int no_dwells;
	std::ifstream inf;
	inf.open(filename);

	if (inf.fail()) {
		inf.clear();
		exit(1);
	}

	std::string line;
	int hh, mm, ss;
	// int dwellVectorIndex = 0;
	int lastGate = 0;
	date d(2010, 1, 1);

	do {
		getline(inf, line);

		if (inf.eof()) {
			inf.close();
			exit(1);
		}
	} while (line.empty());

	std::istringstream line_stream(line);
	line_stream >> no_dwells >> no_platforms;
	TimeSlots &timeSlots = TimeSlots::getInstance();
	_B = new Buses(no_dwells);
	_G = new Gates(no_platforms);
	line_stream.clear();

	for (int gateIter = 0; gateIter < no_platforms; gateIter++) {
		getline(inf, line);
		line_stream.str(line);
		int stationSystemGateId;
		line_stream >> stationSystemGateId;
		(*_G)[gateIter] = new Gate(gateIter, stationSystemGateId);

		do {
			std::string begin, end;
			line_stream >> begin >> end;
			sscanf(begin.c_str(), "%d:%d:%d", &hh, &mm, &ss);
			ptime pt_begin(d,
			               hours(hh) + minutes(mm) + seconds(ss));
			sscanf(end.c_str(), "%d:%d:%d", &hh, &mm, &ss);
			ptime pt_end(d, hours(hh) + minutes(mm) + seconds(ss));
			TimeAvailability *timeWindow =
			    timeSlots.getTimeAvailability(0, pt_begin, pt_end);
			(*_G)[gateIter]->stopAvailableTimes(timeWindow);
		} while (!line_stream.eof());

		line_stream.clear();
	}

	int dwellVectorIndex = 0;

	for (getline(inf, line); !inf.eof() && dwellVectorIndex < no_dwells;
	        getline(inf, line)) {
		if (line == "") {
			continue;
		}

		line_stream.str(line);
		int stationSystemDwellId;
		std::string arrival;
		std::string departure;
		Gates *gates = new Gates();
		line_stream >> stationSystemDwellId;
		line_stream >> arrival;
		line_stream >> departure;
		sscanf(arrival.c_str(), "%d:%d:%d", &hh, &mm, &ss);
		ptime pt_arrival(d, hours(hh) + minutes(mm) + seconds(ss) -minutes(mylag));
		sscanf(departure.c_str(), "%d:%d:%d", &hh, &mm, &ss);
		ptime pt_departure(d, hours(hh) + minutes(mm) + seconds(ss) +minutes(mylag));

		do {
			int gateVectorIndex = 0;
			int stationSystemGateId;
			line_stream >> stationSystemGateId;
			bool yetInserted = false;

			for (Gates::const_iterator gateIter = _G->begin();
			        gateIter != _G->end(); gateIter++) {
				// for (int gateIter = 0; gateIter != lastGate; gateIter++) {
				if ((*gateIter)->gateNumber() ==
				        stationSystemGateId) {
					gateVectorIndex = (*gateIter)->id();
					yetInserted = true;
					break;
				}
			}

			if (!yetInserted) {
				_G->push_back(new
				              Gate(lastGate,
				                   stationSystemGateId));
				gateVectorIndex = lastGate++;
			}

			gates->push_back((*_G)[gateVectorIndex]);
		} while (!line_stream.eof());

		(*_B)[dwellVectorIndex] =
		    new Bus(dwellVectorIndex, stationSystemDwellId, pt_arrival,
		            pt_departure, gates);
		dwellVectorIndex++;
		line_stream.clear();
	}

	// ordina _B per tempo di inizio e fine degli intervalli di sosta
	sort(_B->begin(), _B->end(), BusPtrSort());

	// aggiorna gli id associati ad ogni sosta
	int idx = 0;
	for (Buses::iterator b = _B->begin(); b != _B->end(); b++)
		(*b)->id(idx++);

	// genera il vettore dei tempi
	timeIntervals();
	_numMaximalCliques = 0;
	_maximalCliquesAtGate =
	    new std::vector < std::vector < std::set < int > * > >();
	int k = 0;

	for (Gates::iterator gateIter = _G->begin(); gateIter != _G->end();
	        gateIter++) {
		(*gateIter)->id((*gateIter)->id() + dwellVectorIndex);
		findMaximalCliques(k++);
	}
}

SetModel::SetModel(Buses &B, Gates &G): _B(&B), _G(&G)
{
	TimeSlots &timeSlots = TimeSlots::getInstance();

	// ordina _B per tempo di inizio e fine degli intervalli di sosta
	sort(_B->begin(), _B->end(), BusPtrSort());

	// aggiorna gli id associati ad ogni sosta
	int idx = 0;
	for (Buses::iterator b = _B->begin(); b != _B->end(); b++)
		(*b)->id(idx++);

	// genera il vettore dei tempi
	timeIntervals();
	_numMaximalCliques = 0;
	_maximalCliquesAtGate =
	    new std::vector < std::vector < std::set < int > * > >();

	int k = 0;
	for (Gates::iterator gateIter = _G->begin(); gateIter != _G->end();
	        gateIter++) {
		(*gateIter)->id(k + _B->size());
		findMaximalCliques(k++);
	}
}

SetModel::~SetModel()
{
/*
	for (Intervals::const_iterator i = _I->begin(); i != _I->end(); i++) {
		delete (*i);
	}

	delete _I;
	//for (std::vector <IntervalEndpoint>::const_iterator i = _timeOccupation->begin();
	//      i != _timeOccupation->end(); i++)
	//      delete *i;
	delete _timeOccupation;

	for (std::vector < std::vector < std::set < int > * > >::iterator i =
	            _maximalCliquesAtGate->begin(); i != _maximalCliquesAtGate->end();
	        i++)
		for (std::vector < std::set < int >*>::iterator j = i->begin();
		        j != i->end(); j++) {
			delete *j;
		}

	delete _maximalCliquesAtGate;

	for (Gates::iterator g = _G->begin(); g != _G->end(); g++) {
		delete *g;
	}

	delete _G;

	int intervalIdx = 0;
	for (Buses::iterator dwellIter = _B->begin();
	        dwellIter != _B->end(); dwellIter++) {
		delete *dwellIter;
	}

	delete _B;
	TimeSlots::getInstance().destroy();;
*/
}

void SetModel::timeIntervals()
{
	_timeOccupation = new std::vector < IntervalEndpoint > (2 * _B->size());
	_I = new Intervals(_B->size());
	int intervalIdx = 0;
	int endpointIdx = 0;

	for (Buses::const_iterator dwellIter = _B->begin();
	        dwellIter != _B->end(); dwellIter++) {
		IntervalEndpoint *left =
		    new IntervalEndpoint((**dwellIter).arrival(), leftEP,
		                         (**dwellIter).id());
		IntervalEndpoint *right =
		    new IntervalEndpoint((**dwellIter).departure(), rightEP,
		                         (**dwellIter).id());
		Interval *interval = new Interval(*left, *right);
		(*_timeOccupation)[endpointIdx++] = *left;
		(*_timeOccupation)[endpointIdx++] = *right;
		(*_I)[intervalIdx++] = interval;
	}

	_sortedTimeOccupation = false;
}

/*
 Seleziona solo le soste compatibili col gate ed inseriscile in D;
 esegui la ricerca delle clique di D;
 popola _maximalCliquesAtGate[gate].

 Idea da:
 Parallel Maximal Cliques Algorithms for Interval Graphs with Applications
 Chi Su Wang and Ruay Shiung Chang
*/
void SetModel::findMaximalCliques(int gate)
{
	std::vector < std::set < int >*>maximalCliques;
	maximalCliques.reserve(_I->size());

	if (!_sortedTimeOccupation) {
		sort(_timeOccupation->begin(), _timeOccupation->end(),
		     EndpointLess());
		_sortedTimeOccupation = true;
	}

	int clique = 0;
	std::set < int >*currentClique = new std::set < int >();
	EndPointType previous = leftEP;

	for (std::vector < IntervalEndpoint >::const_iterator i =
	            _timeOccupation->begin(); i != _timeOccupation->end(); i++) {
		if (!(*_B)[i->vertex()]->compatible((*_G)[gate])) {
			continue;
		}

		if (i->endpointType() == leftEP) {
			/* Add to current maximal clique */
			currentClique->insert(i->vertex());
			previous = leftEP;
		} else {
			if (previous == leftEP) {
				std::set < int >*maximalCliquePtr =
				    new std::set < int >(*currentClique);
				maximalCliques.push_back(maximalCliquePtr);
				clique++;
				_numMaximalCliques++;
				currentClique->erase(i->vertex());
			} else {
				currentClique->erase(i->vertex());
				// maximalCliques[clique - 1]->erase(i->vertex());
			}

			previous = rightEP;
		}
	}

	_maximalCliquesAtGate->push_back(maximalCliques);
	// if (maximalCliques.size() <= 1)
	//      std::cout << "Clique di size " << maximalCliques.size() << std::endl;
	delete currentClique;
}

int SetModel::lowerBoundNumberGates()
{

	if (!_sortedTimeOccupation) {
		sort(_timeOccupation->begin(), _timeOccupation->end(),
		     EndpointLess());
		_sortedTimeOccupation = true;
	}

	int gatesLB = 0;
	int max = 0;
	EndPointType previous = leftEP;

	for (std::vector < IntervalEndpoint >::const_iterator i =
	            _timeOccupation->begin(); i != _timeOccupation->end(); i++) {
		if (i->endpointType() == leftEP) {
			gatesLB++;
			previous = leftEP;
		} else {
			if (previous == leftEP) {
				if (max < gatesLB) {
					max = gatesLB;
				}

				gatesLB--;
			} else {
				gatesLB--;
			}

			previous = rightEP;
		}
	}

	// std::cout << "LB on the number of gates:" << max << std::endl;

	return max;
}

std::vector < std::vector < int >*> SetModel::findAllMaximalCliques()
{
	std::vector < std::vector < int >*> maximalCliques;
	maximalCliques.reserve(_I->size());

	if (!_sortedTimeOccupation) {
		sort(_timeOccupation->begin(), _timeOccupation->end(),
		     EndpointLess());
		_sortedTimeOccupation = true;
	}

	int clique = 0;
	std::vector < int >*currentClique = new std::vector < int >();
	EndPointType previous = leftEP;

	for (std::vector < IntervalEndpoint >::const_iterator i =
	            _timeOccupation->begin(); i != _timeOccupation->end(); i++) {

		if (i->endpointType() == leftEP) {
			/* Add to current maximal clique */
			currentClique->push_back(i->vertex());
			previous = leftEP;
		} else {
			if (previous == leftEP) {
				std::vector < int >*maximalCliquePtr =
				    new std::vector < int >(*currentClique);
				maximalCliques.push_back(maximalCliquePtr);
				clique++;
				//_numMaximalCliques++;
				currentClique->erase(std::find(currentClique->begin(), currentClique->end(), i->vertex()));
			} else {
				currentClique->erase(std::find(currentClique->begin(), currentClique->end(), i->vertex()));
				// maximalCliques[clique - 1]->erase(i->vertex());
			}

			previous = rightEP;
		}
	}

	// if (maximalCliques.size() <= 1)
	//      std::cout << "Clique di size " << maximalCliques.size() << std::endl;
	delete currentClique;
	return (maximalCliques);
}

Buses &SetModel::B() const
{
	return *_B;
}

Gates &SetModel::G() const
{
	return *_G;
}

Intervals &SetModel::I() const
{
	return *_I;
}

std::vector < std::vector < std::set < int > * > > & SetModel::maximalCliques() const
{
	return *_maximalCliquesAtGate;
}

unsigned int SetModel::numMaximalCliques() const
{
	return _numMaximalCliques;
}


void SetModel::performances_airo2011(int * solution, unsigned int &used_platform, unsigned int &min_interval_distance, double &cprob_lin, double &cprob_exp, double &mean_distance) const
{
	int gates = _G->size();
	int dwells = _B->size();
	bool * used = new bool [gates];
	std::vector < std::vector <unsigned long int> > platform_endpoint(gates);
	
	for (int j = 0; j < gates; j++)
		used[j] = false;
	
	for (int i = 0; i < dwells ; i++) {
		if (solution[i] >= gates || solution[i] < 0) {
			std::cerr << "Errore: solution[" << i << "] = " << solution[i] << std::endl;
			continue;
		}
		used[solution[i]] = true;
		//std::cout << 0 << std::endl;
		platform_endpoint[solution[i]].push_back(60 * (*_B)[i]->arrival().time_of_day().hours() + (*_B)[i]->arrival().time_of_day().minutes());
		platform_endpoint[solution[i]].push_back(60 * (*_B)[i]->departure().time_of_day().hours() + (*_B)[i]->departure().time_of_day().minutes());
		//std::cout << 1 << std::endl;
	}
	used_platform = 0;
	for (int j = 0; j < gates; j++) {
		if (used[j]) {
			used_platform++;
			sort(platform_endpoint[j].begin(), platform_endpoint[j].end());
		}
	}
	min_interval_distance = mean_distance = 1440;
	unsigned long int total_distance = 0;
	int n = 0;
	for (int j = 0; j < gates; j++) {
		if (!used[j])
			continue;
		unsigned long int a, d;
		size_t points = platform_endpoint[j].size();
		if (points <= 2)
			continue;
		for (int i = 2; i < points -1; i += 2) {
			//std::cout << 4 << std::endl;
			a = platform_endpoint[j][i];
			//std::cout << 5 << std::endl;
			d = platform_endpoint[j][i-1];
			//std::cout << 6 << std::endl;
			long int interval = a-d;
			total_distance += interval;
			if (interval < min_interval_distance)
				min_interval_distance = interval;
			n++;
		}
	}
	if (total_distance != 0)
		mean_distance = (double) total_distance / (double) n;
}
