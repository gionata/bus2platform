/*! \file MathModelColoring.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef MATHMODELCOLORING_H_
#define MATHMODELCOLORING_H_

#include <lpsolve/lp_lib.h>
#include "GraphModel.h"
#include "MathModel.h"

#include <vector>
#include <set>

/**
 *
 */
class MathModelColoring: public MathModel {
public:
	MathModelColoring(GraphModel &graphs);
	~MathModelColoring();

	// virtual bool solveX();
	virtual bool solution(int *&gates) const;
	bool initialSolution(int *startingSolution);

private:
	int _uk_start;

	lprec *createLP(unsigned int _numVars, unsigned int _numConstraints);
	bool setObjectiveFunction();
	bool setAssignmentConstraints();
	bool setIncompatibilityConstraints();
	//bool setLinkConstraints();
	bool setSOS1();
	bool setLbNoGates(int lb);

};

#endif /* MATHMODELCOLORING_H_ */
