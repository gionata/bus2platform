/*! \file MathModelMaxTotalDistance.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef MATHMODELMAXTOTALDISTANCE_H_
#define MATHMODELMAXTOTALDISTANCE_H_

#include <lpsolve/lp_lib.h>
#include "GraphModel.h"
#include "MathModel.h"

#include <vector>
#include <set>

/**
 *
 */
class MathModelMaxTotalDistance: public MathModel {
public:
	MathModelMaxTotalDistance(GraphModel &graphs);
	~MathModelMaxTotalDistance();

	// virtual bool solveX();
	virtual bool solution(int *&gates) const;
	bool initialSolution(int *startingSolution) {
		return true;
	}

private:
	int _yij_start;

	lprec *createLP(unsigned int _numVars, unsigned int _numConstraints);
	bool setObjectiveFunction();
	bool setAssignmentConstraints();
	bool setIncompatibilityConstraints();
	bool setLinkConstraints();
	bool setSOS1();
};

#endif /* MATHMODELMAXTOTALDISTANCE_H_ */
