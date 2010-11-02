/*
 * ListColorTreeSearch.h
 *
 *  Created on: 06/ott/2010
 *      Author: Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef LISTCOLORTREESEARCH_H_
#define LISTCOLORTREESEARCH_H_

#include "SetModel.h"

#include <vector>

class ListColoringTreeSearch {
public:
	ListColoringTreeSearch(SetModel &sets);
	virtual ~ListColoringTreeSearch();

	void ListColoring(unsigned int level);

private:
	SetModel &_sets;
	Buses _sortedBus;
};

#endif /* LISTCOLORTREESEARCH_H_ */
