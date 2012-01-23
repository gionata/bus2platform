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
	S_vector.reserve(_sets.B().size());
	std::copy(_sets.B().begin(), _sets.B().end(), _sortedBus.begin());
	std::sort(_sortedBus.begin(), _sortedBus.end(), arrivalTime());

	std::vector< std::set <int > >::iterator  S_level = S_vector.begin();
	for(Buses::const_iterator vertexItr = _sortedBus.begin(); vertexItr != _sortedBus.end(); vertexItr++) {
		Bus *vertex = *vertexItr;
		const Gates &gates = vertex->gates();
		for(Gates::const_iterator admissibleColorIter = gates.begin(); admissibleColorIter != gates.end(); admissibleColorIter++) {
			int color = (*admissibleColorIter)->vertexIndex();
			S_level->insert( color );
		}
	}
}

ListColoringTreeSearch::~ListColoringTreeSearch()
{
	// TODO Auto-generated destructor stub
}

void ListColoringTreeSearch::ListColoring(unsigned int level, std::set <int > S_colors)
{
	if (level > _sortedBus.size())			//  1
		return;								//  2

	Bus *currentBus = _sortedBus[level];	//  4
	std::set <int > S_prime(S_colors);		//  5
	for (Gates::const_iterator platform = _sets.G().begin(); platform != _sets.G().end(); platform++) {
		if (! currentBus->compatible(*platform))
			continue;

	}
}
