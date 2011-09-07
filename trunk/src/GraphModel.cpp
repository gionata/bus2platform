/*! \file GraphModel.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "GraphModel.h"

#include <cstdio>
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

GraphModel::GraphModel(SetModel &s): _sets(s), _B(s.B()), _G(s.G())
	// , _H_vertex_name(),_C_vertex_name(), , _H_edge_name(),_C_edge_name(), _H_edge_weight(),_C_edge_weight(),
{
	_numDwells = _B.size();
	_numPlatforms = _G.size();
	_numVerticesH = _numDwells + _numPlatforms;
	_compGates = new compatibleGates();
	_Hptr = new GraphH(_numVerticesH);
	_numEdgesH = 0;
	set_graph_H();
	_numCompGates = 0;
	_numVerticesC = _numDwells;
	_Cptr = new GraphC(_numVerticesC);
	_numMaximalCliques = 0;
	_numEdgesC = 0;
	set_graph_C();
	findMaximalCliques();

#ifdef _DEBUG
	debugWriteH();
	debugWriteC();
#endif
}

GraphModel::~GraphModel(void)
{

	///*
	for (unsigned int i = 0; i < _sets.numMaximalCliques(); i++) {
		delete[]_sos1[i];
	}

	delete[]_sos1;
	delete _sos1_corresponding_gate;

	for (unsigned int i = 0; i < _numEdgesC; i++)
		delete (*_compGates)[i];

	delete _compGates;

	delete _Hptr;
	delete _Cptr;
	//*/
}

void
GraphModel::set_graph_H(void)
{
	GraphH &H = *_Hptr;
	//_H_vertex_name = get(vertex_name, H);
	_H_vertex_index = get(vertex_index, H);
	//_H_edge_name = get(edge_name, H);
	_H_edge_index = get(edge_index, H);
	//_H_edge_weight = get(edge_weight, H);

	int begin_gate = _numDwells;
	char *name = new char[16];

	/* Costruisci prima i nodi delle soste, per essere sicuri degli indici con adjacency_matrix */
	int i = 0;
	for (Buses::iterator bitr = _B.begin(); bitr != _B.end();
	        bitr++) {
		// Aggiungi il nome alla sosta
		sprintf(name, "v_%d", (*bitr)->dwellNumber());
		string bname = string(name);
		_H_vertex_name.push_back(bname);
		(*bitr)->vertexIndex(i++);
	}

	// Assegna un nome alle piattaforme
	int gate = begin_gate;
	for (Gates::iterator gitr = _G.begin(); gitr != _G.end(); gitr++) {
		sprintf(name, "c_%d", (*gitr)->gateNumber());
		string gname = string(name);
		_H_vertex_name.push_back(gname);
		(*gitr)->vertexIndex(gate++);
	}

	i = 0;
	_numEdgesH = 0;
	for (Buses::const_iterator bitr = _B.begin(); bitr != _B.end();
	        bitr++, i++) {
		VertexDescriptorH v =
		    vertex(i, H);
		// Per ogni piattaforma compatibile con la sosta *bitr
		for (Gates::const_iterator gitr = (*bitr)->gates().begin();
		        gitr != (*bitr)->gates().end(); gitr++) {
			boost::graph_traits < GraphH >::edge_descriptor e;
			bool inserted;
			// trova l'indice del gate
			gate = (*gitr)->id();
			boost::graph_traits < GraphH >::vertex_descriptor c =
			    vertex(gate, H);
			sprintf(name, "x_%d_%d", (*bitr)->dwellNumber(),
			        (*gitr)->gateNumber());
			tie(e, inserted) =
			    add_edge(v, c,
			             EdgeIndex(_numEdgesH++,
			                       EdgeName(name, EdgeWeight(0))),
			             H);
			_H_edge_name.push_back(name);
			_H_edge_weight.push_back(0);
		}
	}

	delete[]name;
}

void GraphModel::set_graph_C(void)
{
	GraphH &H = *_Hptr;
	// _Cptr = new Graph(_B.size());
	GraphC &C = *_Cptr;
	Buses &B = _B;
	//_C_vertex_name = get(vertex_name, C);
	_C_vertex_index = get(vertex_index, C);
	//_C_edge_name = get(edge_name, C);
	_C_edge_index = get(edge_index, C);
	//_C_edge_weight = get(edge_weight, C);

	/* Copy the bus vertices from H to C */
	int i = 0;

	for (Buses::const_iterator bitr = _B.begin(); bitr != _B.end();
	        bitr++, i++) {
		boost::graph_traits < GraphH >::vertex_descriptor v =
		    vertex(i, H);
		_C_vertex_name.push_back(_H_vertex_name[v]);
	}

	_numEdgesC = 0;

	/* For each gate */
	for (unsigned int gitr = 0; gitr < _numPlatforms; gitr++) {

		/* For each bus that can be assigned to *gitr - in_edge of *gitr - */
		graph_traits < GraphH >::in_edge_iterator in_iH, in_endH;
		boost::graph_traits < GraphH >::vertex_descriptor k =
		    _numDwells + gitr;

		/* for each v_i in B_k */
		for (tie(in_iH, in_endH) = in_edges(k, H); in_iH != in_endH; ++in_iH) {
			graph_traits < GraphH >::edge_descriptor eH = *in_iH;
			boost::graph_traits < GraphH >::vertex_descriptor v_i =
			    source(eH, H);
			graph_traits < GraphH >::in_edge_iterator in_i1H, in_end1H;

			/* For each other bus that can be assigned to *gitr - in_edge of *gitr - */
			for (tie(in_i1H, in_end1H) = in_edges(k, H);
			        in_i1H != in_end1H; ++in_i1H) {
				graph_traits < GraphH >::edge_descriptor e1 =
				    *in_i1H;
				boost::graph_traits <
				GraphH >::vertex_descriptor v_j =
				    source(e1, H);

				if (v_j <= v_i) {
					continue;
				}

				time_period period_i = B[v_i]->occupacyPeriod();
				time_period period_j = B[v_j]->occupacyPeriod();

				if (!period_i.intersects(period_j)) {
					ptime a_i = B[v_i]->arrival();
					ptime a_j = B[v_j]->arrival();
					ptime d_i = B[v_i]->departure();
					ptime d_j = B[v_j]->departure();
					time_duration td;
					double cij = -1.;

					if ((a_i < a_j) && (d_i <= a_j)) {
						td = (a_j - d_i);
					} else if ((a_i > a_j) && (a_i >= d_j)) {
						td = (a_i - d_j);
					}

					cij =
					    60 * td.hours() + td.minutes() +
					    double (td.seconds()) / 60;

					std::pair < boost::graph_traits <
					GraphC >::edge_descriptor,
					       bool > edge_ij = edge(v_i, v_j, C);

					if (edge_ij.second == false) {
						char edge_name_z[50];
						sprintf(edge_name_z, "y_%d_%d",
						        _B[v_i]->dwellNumber(),
						        _B[v_j]->dwellNumber());
						EdgeIndex ep(_numEdgesC,
						             EdgeName
						             (edge_name_z,
						              EdgeWeight(cij)));
						add_edge(v_i, v_j, ep, C);
						_C_edge_name.push_back(edge_name_z);
						_C_edge_weight.push_back(cij);

						//(*_compGates)[_numEdgesC] = new std::vector<std::pair<size_t, size_t> >();
						(*_compGates).push_back(new std::vector<std::pair<size_t, size_t> >());
						std::pair<size_t, size_t> xik_xjk = std::make_pair<size_t, size_t>(_H_edge_index[eH], _H_edge_index[e1]);
						(*_compGates)[_numEdgesC]->push_back(xik_xjk);
						_numCompGates++;

						_numEdgesC++;
					} else {
						((*_compGates)[_C_edge_index[edge_ij.first]])->push_back(std::make_pair<size_t, size_t>(_H_edge_index[eH], _H_edge_index[e1]));
						_numCompGates++;
					}
				}
			}
		}
	}
}

void GraphModel::findMaximalCliques()
{
	//const int xik_start = 1;
	//const int uk_start = numEdgesH() + 1;
	_sos1 = new int *[_sets.numMaximalCliques()];
	_sos1_cardinality = new std::vector < int >(_sets.numMaximalCliques());
	_sos1_corresponding_gate = new std::vector < int >(_sets.numMaximalCliques());

	for (unsigned int i = 0; i < _sets.numMaximalCliques(); i++) {
		_sos1_cardinality->push_back(0);
		_sos1[i] = new int[_numDwells];
	}

	/* Cliques and  SOSs detection */
	_numInterestingCliques = 0;

	/* for each gate */
	for (unsigned int k = 0; k < _numPlatforms; k++) {
		/* For each clique of dwell buses */
		for (vector < set < int >*>::const_iterator incomp_dwells =
		            _sets.maximalCliques()[k].begin();
		        incomp_dwells != _sets.maximalCliques()[k].end();
		        incomp_dwells++) {
			/* if the number of arcs is >= 2 */
			if ((*incomp_dwells)->size() > 1) {
				int sos_index = 0;

				// std::set <int> *currentClique = new std::set <int>();

				/* for each vertex */
				for (set < int >::const_iterator dwell =
				            (*incomp_dwells)->begin();
				        dwell != (*incomp_dwells)->end();
				        dwell++) {
					// indice dell'arco X_${dwell}_${k}
					boost::graph_traits <
					GraphH >::edge_descriptor e;
					bool inserted;
					tie(e, inserted) =
					    edge(*dwell, k + _numDwells,
					         *_Hptr);
					assert(inserted);
					// SETTA I SOS1
					_sos1[_numInterestingCliques][sos_index]
					= _H_edge_index[e];
					(*_sos1_cardinality)
					[_numInterestingCliques]++;
					// currentClique->insert(_H_edge_index[e]);
					sos_index++;
				}

				// (*_mutuallyExclusiveEdgesOnGate)[k].push_back(currentClique);
				(*_sos1_corresponding_gate)[_numInterestingCliques] = k;
				_numInterestingCliques++;
			}
		}
	}
}

GraphH &GraphModel::graphH() const
{
	return *_Hptr;
}

GraphC &GraphModel::graphC() const
{
	return *_Cptr;
}

Buses &GraphModel::B() const
{
	return _B;
}

Gates &GraphModel::G() const
{
	return _G;
}

SetModel &GraphModel::sets() const
{
	return _sets;
}

int **GraphModel::specialOrderedSets1() const
{
	return _sos1;
}
std::vector < int > &GraphModel::sos1_cardinality() const
{
	return *_sos1_cardinality;
}

std::vector < int >&GraphModel::sos1_corresponding_gate() const
{
	return *_sos1_corresponding_gate;
}

compatibleGates &GraphModel::compGates() const
{
	return *_compGates;
}

unsigned int GraphModel::numCompGates() const
{
	return _numCompGates;
}

/*std::map < size_t, std::set < int > * > &GraphModel::linkXiYij() const
{
    // return *_linkXiYij;
}

std::map < size_t, std::set < int > * > &GraphModel::linkXjYij() const
{
    // return *_linkXjYij;
}*/

unsigned int GraphModel::numDwells() const
{
	return _numDwells;
}

unsigned int GraphModel::numPlatforms() const
{
	return _numPlatforms;
}

unsigned int GraphModel::numVerticesH() const
{
	return _numDwells + _numPlatforms;
}

unsigned int GraphModel::numVerticesC() const
{
	return _numDwells;
}

unsigned int GraphModel::numEdgesH() const
{
	return _numEdgesH;
}

unsigned int GraphModel::numEdgesC() const
{
	return _numEdgesC;
}

unsigned int GraphModel::numInterestingCliques() const
{
	return _numInterestingCliques;
}

//#ifdef _DEBUG
void GraphModel::debugWriteH()
{
	string *instance_name;
	ofstream *h_dot;

	instance_name = new string("INDEX_INDEX");
	h_dot = new ofstream(("h_graph_" + *instance_name + ".dot").c_str());
	write_graphviz(*h_dot, *_Hptr,
	               make_label_writer(get(vertex_index, *_Hptr)),
	               make_label_writer(get(edge_index, *_Hptr)));
	h_dot->close();
	delete h_dot;
	delete instance_name;

	instance_name = new string("NAME_NAME");
	h_dot = new ofstream(("h_graph_" + *instance_name + ".dot").c_str());
	write_graphviz(*h_dot, *_Hptr,
	               make_label_writer(get(vertex_name, *_Hptr)),
	               make_label_writer(get(edge_name, *_Hptr)));
	h_dot->close();
	delete h_dot;
	delete instance_name;

	instance_name = new string("NAME_WEIGHT");
	h_dot = new ofstream(("h_graph_" + *instance_name + ".dot").c_str());
	write_graphviz(*h_dot, *_Hptr,
	               make_label_writer(get(vertex_name, *_Hptr)),
	               make_label_writer(get(edge_weight, *_Hptr)));
	h_dot->close();
	delete h_dot;
	delete instance_name;
}

void GraphModel::debugWriteC()
{
	string *instance_name;
	ofstream *c_dot;

	instance_name = new string("INDEX_INDEX");
	c_dot = new ofstream(("c_graph_" + *instance_name + ".dot").c_str());
	write_graphviz(*c_dot, *_Cptr,
	               make_label_writer(get(vertex_index, *_Cptr)),
	               make_label_writer(get(edge_index, *_Cptr)));
	c_dot->close();
	delete c_dot;
	delete instance_name;

	instance_name = new string("NAME_NAME");
	c_dot = new ofstream(("c_graph_" + *instance_name + ".dot").c_str());
	write_graphviz(*c_dot, *_Cptr,
	               make_label_writer(get(vertex_name, *_Cptr)),
	               make_label_writer(get(edge_name, *_Cptr)));
	c_dot->close();
	delete c_dot;
	delete instance_name;

	instance_name = new string("NAME_WEIGHT");
	c_dot = new ofstream(("c_graph_" + *instance_name + ".dot").c_str());
	write_graphviz(*c_dot, *_Cptr,
	               make_label_writer(get(vertex_name, *_Cptr)),
	               make_label_writer(get(edge_weight, *_Cptr)));
	c_dot->close();
	delete c_dot;
	delete instance_name;
}

bool GraphModel::solutionFeasibility(int *&gates, const char *header) const {
		std::ofstream outf("solution_details", std::ios::app);
		unsigned int used_platforms = 0;
		double min_interval = 30000.;
		double conflict_sum = 0.;

		outf << header << std::endl;

		outf << "Gates assignments:";
		for (int i = 0; i < _B.size(); i++)
			outf << "\t" << gates[i];
		outf << std::endl;

		std::vector < std::vector <size_t> > gate_dwell (_G.size());
		bool used[_G.size()];
		for (int i = 0; i < _G.size(); i++) {
			used[i] = false;
		}
		// setta il vettore gate_dwell
		for (int d = 0; d < _numDwells; d++) {
			size_t platform = gates[d];
			if (!used[platform]) {
				used[platform] = true;
				used_platforms++;
				// gate_dwell[platform]= new vector<size_t>();
			}
			gate_dwell[platform].push_back(d);
		}
		outf << "No. used gates:\t" << used_platforms << std::endl;
		outf << "Cij:";
		// controlla, per ogni piattaforma
		for (int p = 0; p < _G.size(); p++) {
			if (!used[p])
				continue;

			// sort gate_dwell[p] by arrival time
			size_t tmp;
			for (int a = 1; a < gate_dwell[p].size(); a++)
				for (int b = gate_dwell[p].size()-1; b >= a; b--)
					if (_B[gate_dwell[p][b-1]]->arrival() > _B[gate_dwell[p][b]]->arrival()) {
						tmp = gate_dwell[p][b-1];
						gate_dwell[p][b-1] = gate_dwell[p][b];
						gate_dwell[p][b] = tmp;
					}
			// per debug
			/*for (int a = 0; a < gate_dwell[p].size(); a++)
				std::cerr << _graphs.B()[gate_dwell[p][a]]->dwellNumber() << " ";
			std::cerr << "\n";*/
			// che ogni sosta
			for (std::vector<size_t>::const_iterator i = gate_dwell[p].begin(); i != gate_dwell[p].end(); i++) {
				// sia assegnabile alla piattaforma
				if (! _B[*i]->compatible(_G[p])) {
					std::cerr << "sosta " << _B[*i]->dwellNumber() << " non compatibile con piattaforma "<< p + 1 << std::endl;
					exit(1);
					return false;
				}
				// non interferisca con nessuna delle soste successive
				if (*i != gate_dwell[p].back()) {
					std::vector<size_t>::const_iterator j = i;
					j++;
					for (; j != gate_dwell[p].end(); j++) {
						if (_B[*i]->occupacyPeriod().intersects(_B[*j]->occupacyPeriod())) {
							std::cerr << "piattaforma " << p + 1 << ", sosta " <<
							     _B[*i]->dwellNumber() << " interferisce con " << _B[*j]->dwellNumber() << std::endl;
							exit(1);
							return false;
						}
					}
				}
			}

			// calcola distanza minima e probabilità di conflitto
			for (int i = 0; i < gate_dwell[p].size() - 1; i++) {
				//time_period period_i = _graphs.B()[i]->occupacyPeriod();
				//time_period period_j = _graphs.B()[i + 1]->occupacyPeriod();

				ptime a_j = _B[gate_dwell[p][i + 1]]->arrival();
				ptime d_i = _B[gate_dwell[p][i]]->departure();
				time_duration td = (a_j - d_i);
				double cij =
					    60 * td.hours() + td.minutes() +
					    (double) td.seconds() / 60;
				conflict_sum += cij;

				if (cij < min_interval)
					min_interval = cij;
				outf << "\t" << cij;
			}
		}
		outf << std::endl;
		outf << "conflict_sum:\t" << conflict_sum << std::endl;
		outf << "min_interval:\t" << min_interval << std::endl;
		outf << std::endl;
		outf.close();

		return true;
	}

//#endif
