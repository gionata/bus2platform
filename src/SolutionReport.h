/*
 * SolutionReport.h
 *
 *  Created on: 11/ott/2010
 *      Author: gim-i3
 */

#ifndef SOLUTIONREPORT_H_
#define SOLUTIONREPORT_H_

#include <vector>
#include <iostream>
#include <string>

enum myType {int_long, fp_double};

class SolutionInformation {
public:
	std::string name;
	std::string description;
	myType type;
	union {
		long int l;
		double d;
	};

	SolutionInformation (char *name, char *description, myType type, void *value);
};

class SolutionReport {
public:
	SolutionReport();
	~SolutionReport();
	void add(SolutionInformation info);
	void print(std::ostream &log);
private:
	std::vector<SolutionInformation> *_dataVecP;
};


#endif /* SOLUTIONREPORT_H_ */
