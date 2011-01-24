/*! \file GraphModel.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef GRAPHMODEL_H_
#define GRAPHMODEL_H_

#include "gap.h"
#include "Bus.h"
#include "Gate.h"
#include "IntervalEndpoint.h"
#include "Interval.h"
#include "SetModel.h"

#include <vector>
#include <set>
#include <map>
#include <utility>
#include <string>

/* Debug */
//#ifdef _DEBUG
#include <boost/graph/graphviz.hpp>
//#endif

class compatibleGatesT {
private:
	unsigned long _key;
	unsigned int _platform;
	unsigned int _i;
	unsigned int _j;
};

typedef std::vector < std::vector<std::pair<size_t, size_t> > * > compatibleGates;

typedef std::vector < std::string > vertex_name_m;
typedef std::vector < std::string > edge_name_m;
typedef std::vector < double > edge_weight_m;

// Ad ogni k associa gli indici delle colonne delle variabili x_{ik} e x_{jk}
/*
class compatibleGates {
public:
	compatibleGates ();
};*/

/**
 *
 */
class GraphModel {
public:
	GraphModel(SetModel &s);
	~GraphModel();
	GraphH &graphH() const;
	GraphC &graphC() const;
	Buses &B() const;
	Gates &G() const;
	SetModel &sets() const;
	int **specialOrderedSets1() const;
	std::vector < int >&sos1_cardinality() const;
	std::vector < int >&sos1_corresponding_gate() const;
//	std::map < size_t, std::set < int > * > &linkXiYij() const;
//	std::map < size_t, std::set < int > * > &linkXjYij() const;
	compatibleGates &compGates() const;
	unsigned int numDwells() const;
	unsigned int numPlatforms() const;
	unsigned int numVerticesH() const;
	unsigned int numVerticesC() const;
	unsigned int numEdgesH() const;
	unsigned int numEdgesC() const;
	unsigned int numInterestingCliques() const;
	unsigned int numCompGates() const;

	vertex_name_m H_vertex_name() const {
		return _H_vertex_name;
	}
	vertex_name_m C_vertex_name() const {
		return _C_vertex_name;
	}
	edge_name_m H_edge_name() const {
		return _H_edge_name;
	}
	edge_name_m C_edge_name() const {
		return _C_edge_name;
	}
	edge_weight_m C_edge_weight() {
		return _C_edge_weight;
	}
	edge_weight_m H_edge_weight() {
		return _H_edge_weight;
	}
	boost::property_map < GraphH, boost::vertex_index_t >::type H_vertex_index() const {
		return _H_vertex_index;
	}
	boost::property_map < GraphC, boost::vertex_index_t >::type C_vertex_index() const {
		return _C_vertex_index;
	}
	boost::property_map < GraphC, boost::edge_index_t >::type C_edge_index() {
		return _C_edge_index;
	}
	boost::property_map < GraphH, boost::edge_index_t >::type H_edge_index() {
		return _H_edge_index;
	}

	/* Debug related methods */
//#ifdef _DEBUG
	void debugWriteH();
	void debugWriteC();
//#endif



private:
	/* Sets */
	SetModel &_sets;
	/* Set (vector) of bus dwells */
	Buses &_B;
	/* Set of platforms */
	Gates &_G;

	/* Sets cardinality */
	unsigned int _numDwells;
	unsigned int _numPlatforms;

	/* Graphs */
	GraphH *_Hptr;
	GraphC *_Cptr;

	/* Graphs orders and sizes */
	unsigned int _numEdgesH;
	unsigned int _numVerticesH;
	unsigned int _numEdgesC;
	unsigned int _numVerticesC;

	/* Graphs properties - Cosi' l'accesso è lento*/
	/*
	boost::property_map < GraphH, boost::vertex_name_t >::type _H_vertex_name;
	boost::property_map < GraphC, boost::vertex_name_t >::type _C_vertex_name;
	boost::property_map < GraphH, boost::vertex_index_t >::type _H_vertex_index;
	boost::property_map < GraphC, boost::vertex_index_t >::type _C_vertex_index;
	boost::property_map < GraphH, boost::edge_name_t >::type _H_edge_name;
	boost::property_map < GraphC, boost::edge_name_t >::type  _C_edge_name;
	boost::property_map < GraphH, boost::edge_index_t >::type _H_edge_index;
	boost::property_map < GraphC, boost::edge_index_t >::type _C_edge_index;
	boost::property_map < GraphH, boost::edge_weight_t >::type _H_edge_weight;
	boost::property_map < GraphC, boost::edge_weight_t >::type _C_edge_weight;
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

	/* Vectors for the arcs belonging to maximal cliques */
	std::vector < int > _InterestingCliqueGate;
	int _numInterestingCliques;
	int **_sos1;
	std::vector < int > *_sos1_cardinality;
	std::vector < int > *_sos1_corresponding_gate;
	//std::map < size_t, std::set < int > * > *_linkXiYij;
	//std::map < size_t, std::set < int > * > *_linkXjYij;

	/* The *_mutuallyExclusiveEdgesOnGate vector contains the set of mutually incompatible buses dwells.
	   The relation is the set of edges indexes:
	   *_mutuallyExclusiveEdgesOnGate[gate][cliqueNumber] -> the set of incompatible edgesIndexes
	 */
	// std::vector<std::vector<std::set <int> * > > *_mutuallyExclusiveEdgesOnGate;
	unsigned int _numMaximalCliques;


	compatibleGates *_compGates;
	unsigned int _numCompGates;

	/* Methods */

	void set_graph_H(void);
	void set_graph_C(void);
	void findMaximalCliques();

};


#endif /* GRAPHMODEL_H_ */
