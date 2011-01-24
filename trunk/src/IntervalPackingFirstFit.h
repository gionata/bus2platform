/*! \file IntervalPackingFirstFit.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef IntervalPackingFirstFit_H_
#define IntervalPackingFirstFit_H_

#include "SetModel.h"
#include "GraphModel.h"
#include "SolutionAlgorithm.h"

#include <vector>

/**
 *
 */
class IntervalPackingFirstFit: public SolutionAlgorithm {
public:
	IntervalPackingFirstFit(SetModel &sets, GraphModel &graphs);
	~IntervalPackingFirstFit();

	virtual bool solveX();
	virtual bool solution(int *&gates) const;
private:
	SetModel &_sets;
	GraphModel &_graphs;
};

#endif /* IntervalPackingFirstFit_H_ */
