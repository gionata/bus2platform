/*! \file MathModelBPsingle.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "MathModelBPsingle.h"

using namespace std;
using namespace boost;

MathModelBPsingle::MathModelBPsingle(GraphModel &graphs): MathModel(graphs)
{

	_numVars = _graphs.numEdgesH() + _graphs.numPlatforms();
	_numConstraints = _numDwells +     /* Assignment */
				   _graphs.numInterestingCliques() +    /* Incompatibility */
				   _graphs.numEdgesH() /* U_k linx to X_ik */
				   + 1;                /* LB */
	_uk_start = _graphs.numEdgesH() + 1;

	_row_values = new double[_numVars + 1];
	_row_values[0] = 1.0;

	for (int i = 1; i < _numVars + 1; i++) {
		_row_values[i] = 1.0;
	}

	_lp = createLP(_numVars, _numConstraints);
	setObjectiveFunction();
	setAssignmentConstraints();
	setIncompatibilityConstraints();
	setLinkConstraints();
	int lb = _graphs.sets().lowerBoundNumberGates();
	setLbNoGates(lb);
	setSOS1();
}

MathModelBPsingle::~MathModelBPsingle()
{
	delete[]_row_values;
}

lprec *MathModelBPsingle::createLP(unsigned int numVars,
							unsigned int numConstraints)
{
	if ((_lp = make_lp(numConstraints, numVars)) == NULL) {
		exit(1);
	}

	set_lp_name(_lp, "List-coloring_extended");
	set_minim(_lp);

	graph_traits < GraphH >::edge_iterator ei, edge_end;
	for (tie(ei, edge_end) = edges(_graphs.graphH()); ei != edge_end; ++ei) {
		size_t xik_idx = _H_edge_index[*ei];
		int variableIndex = xik_idx + _xik_start;
		(*_assignment)[xik_idx].first =
			source(*ei, _graphs.graphH());
			(*_assignment)[xik_idx].second =
				target(*ei, _graphs.graphH());
		set_binary(_lp, variableIndex, TRUE);
		//strcpy(_col_or_row_name, _H_edge_name[*ei].c_str());
		strcpy(_col_or_row_name, _H_edge_name[xik_idx].c_str());
		set_col_name(_lp, variableIndex, _col_or_row_name);
	}

	for (int i = _uk_start; i <= _numVars; i++) {
		set_obj(_lp, i, i - _uk_start + 1);
		set_binary(_lp, i, TRUE);
		sprintf(_col_or_row_name, "u_%d", i - _uk_start + 1);
		set_col_name(_lp, i, _col_or_row_name);
	}

	set_row_name(_lp, 0, "PlatformXdistance");

	return _lp;
}

bool MathModelBPsingle::setObjectiveFunction()
{
	// Already done in createLP()
	return true;
}

bool MathModelBPsingle::setAssignmentConstraints()
{

	/* for each dwell */
	for (unsigned int i = 0; i < _numDwells; i++) {
		sprintf(_col_or_row_name, "dwell_%d", i + 1);
		set_row_name(_lp, i + 1, _col_or_row_name);
		graph_traits < GraphH >::out_edge_iterator out_i, out_end;

		for (tie(out_i, out_end) = out_edges(i, _graphs.graphH());
				out_i != out_end; ++out_i) {
			set_mat(_lp, i + 1, _H_edge_index[*out_i] + 1, 1.0);
		}

		set_constr_type(_lp, i + 1, EQ);
		set_rh(_lp, i + 1, 1.0);
	}

	return true;
}

bool MathModelBPsingle::setIncompatibilityConstraints()
{

	int row_no = _numDwells;

	for (int soss = 0; soss < _numInterestingCliques; soss++) {
		row_no++;
		sprintf(_col_or_row_name, "incomp_clique_%d", soss + 1);
		set_rowex(_lp, row_no, _sos1Cardinality[soss], _row_values,
				_colno[soss]);
		set_constr_type(_lp, row_no, LE);
		set_rh(_lp, row_no, 1.0);
		set_row_name(_lp, row_no, _col_or_row_name);
	}

	return true;
}

bool MathModelBPsingle::setLinkConstraints()
{

	int row_no = _numDwells + _numInterestingCliques;
	/* for each gate */
	for (unsigned int k = 0; k < _graphs.numPlatforms(); k++) {
		int k1 = k + _numDwells;
		graph_traits < GraphH >::in_edge_iterator in_i, in_end;
		/* for each node i | exists (i, k ) \in H(B, G, E) */
		for (tie(in_i, in_end) = in_edges(k1, _graphs.graphH());
				in_i != in_end; ++in_i) {
			row_no++;
			graph_traits < GraphH >::edge_descriptor e = *in_i;
			set_mat(_lp, row_no, _H_edge_index[e] + 1, 1.0);
			set_mat(_lp, row_no, _uk_start + k, -1.0);
			set_constr_type(_lp, row_no, LE);
			set_rh(_lp, row_no, 0.0);
			VertexDescriptorH i = source(e, _graphs.graphH());
			sprintf(_col_or_row_name, "link_x_%d_%d_to_u", (_graphs.B()[_H_vertex_index[i]])->dwellNumber(), k + 1);
			set_row_name(_lp, row_no, _col_or_row_name);
		}
	}

	return true;
}

bool MathModelBPsingle::setSOS1()
{
	int index;

	for (int soss = 0; soss < _numInterestingCliques; soss++) {

		sprintf(_col_or_row_name, "SOS_%d", soss + 1);
		index =
			add_SOS(_lp, _col_or_row_name, 1, 1, _sos1Cardinality[soss],
				   _colno[soss], NULL);


	}
	return true;
}

bool MathModelBPsingle::setLbNoGates(int lb)
{
	int row_no = _numConstraints;
	int *columns_no = new int[_numVars - _uk_start + 1];

	for (int i = _uk_start; i <= _numVars; i++) {
		columns_no[i - _uk_start] = i;
	}

	if (set_rowex
			(_lp, row_no, _graphs.numPlatforms(), _row_values,
			 columns_no) == FALSE) {
		delete[]columns_no;
		return false;
	}

	delete[]columns_no;
	set_constr_type(_lp, row_no, GE);
	set_rh(_lp, row_no, double (lb));
	sprintf(_col_or_row_name, "lower_bound");
	set_row_name(_lp, row_no, _col_or_row_name);

	return true;
}

/*
unsigned char MathModelBPsingle::writeModelLP_solve(char *filename)
{
	return write_lp(_lp, filename);
}

unsigned char MathModelBPsingle::writeModelCPLEX(char *filename)
{
	set_XLI(_lp, "xli_CPLEX");
	return write_XLI(_lp, filename, 0, FALSE);
}
*/

bool MathModelBPsingle::solution(int *&gates) const
{
	double *sol = new double[_numVars];
	gates = new int[_numDwells];

	if (get_variables(_lp, sol) == FALSE) {
		return false;
	}

	for (int i = 0; i < _uk_start - 1; i++)
		if (sol[i] >= 0.998) {
			// se si usa una adjacency_list
			gates[(*_assignment)[i].first] =
				(*_assignment)[i].second - _numDwells;
			//cout << /*get_col_name(_lp, i+1) << "  " <<*/ _graphs.sets().B()[(*_assignment)[i].first]->dwellNumber() << "; " << _graphs.sets().G()[(*_assignment)[i].second - _numDwells]->gateNumber() << endl;
			//cout << /*get_col_name(_lp, i+1) << "  " <<*/ (*_assignment)[i].first << "; " << (*_assignment)[i].second << endl;
		}
	// cout << endl;
	delete []sol;

	return true;
}

bool MathModelBPsingle::initialSolution(int *startingSolution)
{
	double *varValues = new double[1 + _numVars];
	int *basisvector = new int[1 + _numVars + _numConstraints];

	for (int i = 0; i < 1 + _numVars; i++)
		varValues[i] = 0.0;

	bool status;
	for (unsigned int dwell = 0; dwell < _graphs.numDwells(); dwell++) {
		size_t i = _graphs.B()[dwell]->vertexIndex();
		unsigned int gate = startingSolution[dwell];
		size_t k = _graphs.G()[gate]->vertexIndex();
		size_t x_ik = _H_edge_index[edge(i, k, _H).first] + _xik_start;
		varValues[x_ik] = 1.;
		// cout << "(k = " << k << ", idx = " << k - _numDwells + _uk_start << "vars = " << _numVars << ")" << endl;
		varValues[k - _numDwells + _uk_start] = 1.;
	}

	if ( guess_basis(_lp, varValues, basisvector) == TRUE) {
		if (set_basis(_lp, basisvector, TRUE) == TRUE) {
			status = true;
		}
	} else {
		status = false;
	}

	delete []varValues;
	delete []basisvector;

	return status;
}
