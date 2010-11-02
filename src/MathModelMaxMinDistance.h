/*! \file MathModelMaxMinDistance.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef MATHMODELMAXMINDISTANCE_H_
#define MATHMODELMAXMINDISTANCE_H_

#include <lpsolve/lp_lib.h>
#include "MathModel.h"
#include "GraphModel.h"
#include "Gate.h"

#include <vector>
#include <set>

/**
 *
 */
class MathModelMaxMinDistance: public MathModel {
public:
	MathModelMaxMinDistance(GraphModel &graphs);
	~MathModelMaxMinDistance();

	//virtual bool solveX();
	virtual bool solution(int *&gates) const;
	bool initialSolution(int *startingSolution);

private:

	int _yij_start;
	int _m_start;
	double _bigM;
	double _linkrow[3];
	int _linkcolno[3];

	lprec *createLP(unsigned int _numVars, unsigned int _numConstraints);

	bool setObjectiveFunction();
	bool setAssignmentConstraints();
	bool setIncompatibilityConstraints();
	bool setLinkConstraints();
	bool setSOS1();
	bool setLinkMConstraints();

};

#endif /* MATHMODELMAXMINDISTANCE_H_ */
