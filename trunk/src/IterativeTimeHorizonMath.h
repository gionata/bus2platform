/*! \file IterativeTimeHorizonMath.h
 *  \brief Solution by iterative solving math model
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef ITERATIVE_TIME_HORIZON_MATH_H_
#define ITERATIVE_TIME_HORIZON_MATH_H_

#include "gap.h"
#include "Bus.h"
#include "Gate.h"
#include "SetModel.h"
#include "GraphModel.h"
#include "SolutionAlgorithm.h"
#include "MathModel.h"

#include "MathModelBP.h"
#include "MathModelBPsingle.h"
#include "MathModelColoring.h"
#include "MathModelMaxMinDistance.h"
#include "MathModelMinPConflict.h"

#include <vector>
#include <set>

/**
 *
 */
class IterativeTimeHorizonMath: public SolutionAlgorithm {
public:
	IterativeTimeHorizonMath(GraphModel &graphs); /*  */
	~IterativeTimeHorizonMath();

	bool solveX();
	bool solution(int *&gates) const;
	bool initialSolution(int *startingSolution);

private:
	ptime _opening, _closing;
	GraphH &_H;
	GraphC &_C;
	GraphModel &_graphs;
	std::vector < std::pair < int, int > > *_assignment;


	void findTotalHorizon();
};

#endif
