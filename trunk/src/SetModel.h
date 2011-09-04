/*! \file SetModel.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef SETMODEL_H_
#define SETMODEL_H_

#include "Bus.h"
#include "Gate.h"
#include "Interval.h"

#include <vector>
#include <set>

#ifdef _B
#	undef _B
#endif

/**
 *
 */
class SetModel {
public:
	SetModel(char *filename);
	SetModel(Buses &B, Gates &G);
	~SetModel();

	Buses &B() const;
	Gates &G() const;
	Intervals &I() const;
	std::vector < std::vector < std::set < int > * > > & maximalCliques() const;
	unsigned int numMaximalCliques() const;
	int lowerBoundNumberGates();
	void performances_airo2011(int * solution, unsigned int used_platform, unsigned int min_interval_distance, double cprob_lin, double cprob_exp) const;

private:
	Buses *_B;
	Gates *_G;
	std::vector < IntervalEndpoint > *_timeOccupation;
	Intervals *_I;
	std::vector < std::vector < std::set < int > * > > *_maximalCliquesAtGate;
	unsigned int _numMaximalCliques;
	bool _sortedTimeOccupation;
	void findMaximalCliques(int gate);
	void timeIntervals();
};

#endif /* SETMODEL_H_ */
