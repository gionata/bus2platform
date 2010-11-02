/*! \file IntervalPackingFinishFirst.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef INTERVALPACKINGFINISHFIRST_H_
#define INTERVALPACKINGFINISHFIRST_H_

#include "SetModel.h"
#include "GraphModel.h"
#include "SolutionAlgorithm.h"

#include <vector>

/**
 *
 */
class IntervalPackingFinishFirst: public SolutionAlgorithm {
public:
	IntervalPackingFinishFirst(SetModel &sets, GraphModel &graphs);
	~IntervalPackingFinishFirst();

	virtual bool solveX();
	virtual bool solution(int *&gates) const;
private:
	SetModel &_sets;
	GraphModel &_graphs;
};

#endif /* INTERVALPACKINGFINISHFIRST_H_ */
