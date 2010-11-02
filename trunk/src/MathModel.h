/*! \file MathModelBP.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef MATHMODEL_H_
#define MATHMODEL_H_

#include <lpsolve/lp_lib.h>
#include "GraphModel.h"
#include "gap.h"
#include "SolutionAlgorithm.h"

#include <boost/graph/graphviz.hpp>
#include <boost/property_map/property_map.hpp>

#include <vector>
#include <set>

class MathModel: public SolutionAlgorithm {
public:

    MathModel(GraphModel &graphs):
        _H(graphs.graphH()),
        _C(graphs.graphC()),
        _sos1Cardinality(graphs.
                         sos1_cardinality()),
        _numDwells(graphs.numDwells()),
        _graphs(graphs),
        _sos1 (_graphs.specialOrderedSets1()),
        _numInterestingCliques(_graphs.numInterestingCliques()),
    	_total_iter(0),
    	_total_nodes(0),
    	_objectiveFunction(0)

    {
    	/*
        _H_vertex_name = get(boost::vertex_name, _H);
        _H_vertex_index = get(boost::vertex_index, _H);
        _H_edge_name = get(boost::edge_name, _H);
        _H_edge_index = get(boost::edge_index, _H);
        _H_edge_weight = get(boost::edge_weight, _H);
        _C_vertex_index = get(boost::vertex_index, _C);
        _C_edge_name = get(boost::edge_name, _C);
        _C_edge_index = get(boost::edge_index, _C);
        _C_edge_weight = get(boost::edge_weight, _C);
        */

        _H_vertex_name = _graphs.H_vertex_name();
        _H_vertex_index = _graphs.H_vertex_index();
        _H_edge_name = _graphs.H_edge_name();
        _H_edge_index = _graphs.H_edge_index();
        _H_edge_weight = _graphs.H_edge_weight();
        _C_vertex_index = _graphs.C_vertex_index();
        _C_edge_name = _graphs.C_edge_name();
        _C_edge_index = _graphs.C_edge_index();
        _C_edge_weight = _graphs.C_edge_weight();


        _xik_start = 1;
        _col_or_row_name = new char[100];
        _colno = new int *[_numInterestingCliques];
        for (int soss = 0; soss < _numInterestingCliques; soss++) {
            int count = _sos1Cardinality[soss];
            _colno[soss] = new int[count];

            for (int i = 0; i < count; i++) {
                _colno[soss][i] = _sos1[soss][i] + 1;
            }
        }
		_assignment = new std::vector < std::pair < int, int > >(_graphs.numEdgesH());
    }
    ~MathModel() {
        delete[]_col_or_row_name;

        for (int soss = 0; soss < _numInterestingCliques; soss++) {
            delete[]_colno[soss];
        }

        delete [] _colno;
        delete _assignment;
		delete_lp(_lp);
    }

    bool solveX()
    {
    	/*
    	   set_presolve(_lp,
    	   PRESOLVE_ROWS | PRESOLVE_COLS | PRESOLVE_LINDEP |
    	   PRESOLVE_REDUCEGCD | PRESOLVE_PROBEREDUCE | PRESOLVE_BOUNDS | PRESOLVE_DUALS,
    	   1000
    	   );
    	 */
    	unsigned char status;
    	_clock_begin = clock();
    	status = solve(_lp);
    	_clock_end = clock();

    	_objectiveFunction = get_objective(_lp);
    	_total_iter = get_total_iter(_lp);
    	_total_nodes = get_total_nodes(_lp);

    	if (status == OPTIMAL || status == SUBOPTIMAL || status == FEASFOUND)
    		_solved = true;
    	else
    		_solved = false;

    	if (status == FALSE) {
    		return false;
    	}

    	return true;
    }

	unsigned char writeModelLP_solve(char *filename) {
		return write_lp(_lp, filename);
	}
    unsigned char writeModelCPLEX(char *filename) {
		set_XLI(_lp, "xli_CPLEX");
		return write_XLI(_lp, filename, 0, FALSE);
	}
	void setTimeout(long second) {
		set_timeout(_lp, second); 
	}
	long long int totalIter() { return _total_iter = get_total_iter(_lp); }
	long long int totalNodes() {return _total_nodes = get_total_nodes(_lp); }
	double objectiveFunction() { return _objectiveFunction = get_objective(_lp); }
	void verbose(int level) { set_verbose(_lp, level); }

	// TODO scegliere se metterlo qui o in SolutionAlgorithm
	virtual bool initialSolution(int *startingSolution) = 0;

protected:
    GraphH &_H;
    GraphC &_C;
    GraphModel &_graphs;

    lprec *_lp;
    int _numVars;
    int _numConstraints;

    char *_col_or_row_name;
    const unsigned int _numDwells;
    int _xik_start;
    int **_sos1;
    int **_colno;
    int _numInterestingCliques;
    double *_row_values;
    std::vector < int >&_sos1Cardinality;
    std::vector < std::pair < int, int > > *_assignment;

    int _currentRow;

	long long int _total_iter;
	long long int _total_nodes;
	double _objectiveFunction;

/* Graphs properties */
	/*
boost::property_map < GraphH,
	 boost::vertex_name_t >::type _H_vertex_name;
boost::property_map < GraphC,
		 boost::vertex_name_t >::type _C_vertex_name;
boost::property_map < GraphH,
	 boost::vertex_index_t >::type _H_vertex_index;
boost::property_map < GraphC,
		 boost::vertex_index_t >::type _C_vertex_index;
boost::property_map < GraphH, boost::edge_name_t >::type _H_edge_name;
boost::property_map < GraphC, boost::edge_name_t >::type  _C_edge_name;
boost::property_map < GraphH, boost::edge_index_t >::type _H_edge_index;
boost::property_map < GraphC, boost::edge_index_t >::type _C_edge_index;
boost::property_map < GraphH,
	 boost::edge_weight_t >::type _H_edge_weight;
boost::property_map < GraphC,
		 boost::edge_weight_t >::type _C_edge_weight;
*/
	vertex_name_m _H_vertex_name;
	vertex_name_m _C_vertex_name;
	boost::property_map < GraphH, boost::vertex_index_t >::type _H_vertex_index;
	boost::property_map < GraphC, boost::vertex_index_t >::type _C_vertex_index;
	edge_name_m _H_edge_name;
	edge_name_m _C_edge_name;
	boost::property_map < GraphH, boost::edge_index_t >::type _H_edge_index;
	boost::property_map < GraphC, boost::edge_index_t >::type _C_edge_index;
	edge_weight_m _H_edge_weight;
	edge_weight_m _C_edge_weight;

    virtual lprec *createLP(unsigned int _numVars, unsigned int _numConstraints) = 0;
    virtual bool setObjectiveFunction() = 0;
    virtual bool setAssignmentConstraints() = 0;
    virtual bool setIncompatibilityConstraints() = 0;
    virtual bool setSOS1() = 0;
};

#endif /* MATHMODEL_H */
