/*! \file MathModelBPsingle.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef MATHMODELBPSINGLE_H_
#define MATHMODELBPSINGLE_H_

#include <lpsolve/lp_lib.h>
#include "GraphModel.h"
#include "MathModel.h"

#include <vector>
#include <set>

/**
 *
 */
class MathModelBPsingle: public MathModel {
public:
	MathModelBPsingle(GraphModel &graphs);
	~MathModelBPsingle();
//	virtual bool solveX();
	virtual bool solution(int *&gates) const;
	bool initialSolution(int *startingSolution);

private:
	int _uk_start;

	
	lprec *createLP(unsigned int _numVars, unsigned int _numConstraints);
	bool setObjectiveFunction();
	bool setAssignmentConstraints();
	bool setIncompatibilityConstraints();
	bool setLinkConstraints();
	bool setSOS1();
	bool setLbNoGates(int lb);
	
};

#endif /* MATHMODELBPSINGLE_H_ */
