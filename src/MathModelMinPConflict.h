/*! \file MathModelMinPConflict.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef MATHMODELMINPCONFLICT_H_
#define MATHMODELMINPCONFLICT_H_

#include <lpsolve/lp_lib.h>
#include "GraphModel.h"
#include "MathModel.h"
#include "SolutionReport.h"

#include <vector>
#include <set>

/**
 *
 */
class MathModelMinPConflict: public MathModel {
public:
	MathModelMinPConflict(GraphModel &graphs);
	~MathModelMinPConflict();

	//virtual bool solveX();
	virtual bool solution(int *&gates) const;
	bool initialSolution(int *startingSolution);

protected:
	double _D;
	double _d;
	SolutionReport _sr;

private:
	int _yij_start;

	lprec *createLP(unsigned int _numVars, unsigned int _numConstraints);

	bool setObjectiveFunction();
	bool setAssignmentConstraints();
	bool setIncompatibilityConstraints();
	bool setLinkConstraints();
	bool setSOS1();
};

#endif /* MATHMODELMINPCONFLICT_H_ */
