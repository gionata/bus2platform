/*
 * SolutionReport.cpp
 *
 *  Created on: 11/ott/2010
 *      Author: gim-i3
 */

#include "SolutionReport.h"

using namespace std;

SolutionInformation::SolutionInformation (char *name, char *description, myType type, void *value)
{
	this->name = string(name);
	this->description = string (description);
	this->type = type;
	if (type == int_long)
		this->l = *((long int *) value);
	else if (type == fp_double)
		this->d = *((double *) value);
}

SolutionReport::SolutionReport()
{
	// TODO Auto-generated constructor stub
	_dataVecP = new vector<SolutionInformation>();
}

SolutionReport::~SolutionReport()
{
	// TODO Auto-generated destructor stub
	/*for (vector<SolutionInformation>::iterator si = _dataVecP->begin();
			si != _dataVecP->end(); si++)
		delete (&(*si));*/
	delete _dataVecP;
}

void SolutionReport::add(SolutionInformation info)
{
	_dataVecP->push_back(info);
}

void SolutionReport::print(ostream &log)
{
	for (vector<SolutionInformation>::iterator si = _dataVecP->begin();
	        si != _dataVecP->end(); si++) {
		log << si->name << "\t" << si->description << "\t" << si->type << "\t";
		if (si->type == int_long)
			log << si->l;
		else if (si->type == fp_double)
			log << si->d;
		log << endl;
	}
}
