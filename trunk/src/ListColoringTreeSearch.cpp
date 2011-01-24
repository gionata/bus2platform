/*
 * ListColorTreeSearch.cpp
 *
 *  Created on: 06/ott/2010
 *      Author: gim-i3
 */

#include "ListColoringTreeSearch.h"
#include <algorithm>

ListColoringTreeSearch::ListColoringTreeSearch(SetModel &sets) : _sets(sets)
{
	// TODO Auto-generated constructor stub
	_sortedBus.reserve(_sets.B().size());
	std::copy(_sets.B().begin(), _sets.B().end(), _sortedBus.begin());
	std::sort(_sortedBus.begin(), _sortedBus.end(), arrivalTime());
}

ListColoringTreeSearch::~ListColoringTreeSearch()
{
	// TODO Auto-generated destructor stub
}

void ListColoringTreeSearch::ListColoring(unsigned int level)
{
	if (level > _sortedBus.size())
		return;

	Bus *currentBus = _sortedBus[level];
	// manca S' = S
	for (Gates::const_iterator platform = _sets.G().begin(); platform != _sets.G().end(); platform++) {
		if (! currentBus->compatible(*platform))
			continue;

	}
}
