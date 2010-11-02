/*! \file IntervalPackingFirstFit.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef IntervalPackingFirstFit_H_
#define IntervalPackingFirstFit_H_

#include "SetModel.h"
#include "GraphModel.h"

#include <vector>

/**
 *
 */
class IntervalPackingFirstFit {
public:
	IntervalPackingFirstFit(SetModel &sets, GraphModel &graphs);
	~IntervalPackingFirstFit();
private:
	SetModel &_sets;
	GraphModel &_graphs;
};

#endif /* IntervalPackingFirstFit_H_ */
