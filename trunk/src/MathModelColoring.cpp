/*! \file MathModelColoring.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "MathModelColoring.h"

using namespace std;
using namespace boost;

MathModelColoring::MathModelColoring(GraphModel &graphs): MathModel(graphs)
{

	_numVars = _graphs.numEdgesH() + _graphs.numPlatforms();
	_numConstraints = _numDwells +     /* Assignment */
				   _graphs.numInterestingCliques() +    /* Incompatibility */
				   //_graphs.numPlatforms() +   /* U_k linx to X_ik */
				   1;               /* lower bound sul numero di gates aperti */
				   /* 1 -- upper bound */
				   /* |V| -- ( \sum_{j=1}^g u_j ) >= ( \sum_{j=1}^g j * x_{ij} ) */
	_uk_start = _graphs.numEdgesH() + 1;


	_row_values = new double[_numVars + 1];
	_row_values[0] = 1.0;

	for (int i = 1; i < _numVars + 1; i++) {
		_row_values[i] = 1.0;
	}

	_column_numbers = new int[_numVars + 1];

	_lp = createLP(_numVars, _numConstraints);
	setObjectiveFunction();
	setAssignmentConstraints();
	setIncompatibilityConstraints();
	//setLinkConstraints();
	int lb = _graphs.sets().lowerBoundNumberGates();
	setLbNoGates(lb);
	// setUbNoGates(ub);
	// setReduceFractionalSols();
	setSOS1();
	set_add_rowmode(_lp, FALSE);
}

MathModelColoring::~MathModelColoring()
{
	delete[]_row_values;
	delete[]_column_numbers;
}

lprec *MathModelColoring::createLP(unsigned int numVars, unsigned int numConstraints)
{
	if ((_lp = make_lp(numConstraints, numVars)) == NULL) {
		exit(1);
	}

	set_add_rowmode(_lp, TRUE);
#ifndef _NO_LP_NAMES
	set_lp_name(_lp, "BinPacking");
#endif

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

#ifndef _NO_LP_NAMES
		strcpy(_col_or_row_name, _H_edge_name[xik_idx].c_str());
		set_col_name(_lp, variableIndex, _col_or_row_name);
#endif
	}

	for (int i = _uk_start; i <= _numVars; i++) {
		set_obj(_lp, i, i - _uk_start + 1);
		set_binary(_lp, i, TRUE);

#ifndef _NO_LP_NAMES
		sprintf(_col_or_row_name, "u_%d", i - _uk_start + 1);
		set_col_name(_lp, i, _col_or_row_name);
#endif
	}

#ifndef _NO_LP_NAMES
	set_row_name(_lp, 0, "PlatformXdistance");
#endif

	return _lp;
}

bool MathModelColoring::setObjectiveFunction()
{
	// Already done in createLP()
	return true;
}

bool MathModelColoring::setAssignmentConstraints()
{

	/* for each dwell */
	for (unsigned int i = 0; i < _numDwells; i++) {
#ifndef _NO_LP_NAMES
		sprintf(_col_or_row_name, "dwell_%d", i + 1);
		set_row_name(_lp, i + 1, _col_or_row_name);
#endif
		graph_traits < GraphH >::out_edge_iterator out_i, out_end;

		unsigned count = 0;
		for (tie(out_i, out_end) = out_edges(i, _graphs.graphH());
				out_i != out_end; ++out_i) {
			_column_numbers[count++] = _H_edge_index[*out_i] + 1;
		}

		set_rowex(_lp, i + 1, count, _row_values, _column_numbers);
		set_constr_type(_lp, i + 1, EQ);
		set_rh(_lp, i + 1, 1.0);
	}

	return true;
}

bool MathModelColoring::setIncompatibilityConstraints()
{

	int row_no = _numDwells;

	for (int soss = 0; soss < _numInterestingCliques; soss++) {
		row_no++;
		// copia _colno[soss] in un nuovo array di _sos1Cardinality[soss] + 1
		int columns_numbers_uk[_sos1Cardinality[soss]+1];
		for (int i = 0; i <_sos1Cardinality[soss]; i++)
		  columns_numbers_uk[i] = _colno[soss][i];
		columns_numbers_uk[_sos1Cardinality[soss]] = _uk_start + _graphs.sos1_corresponding_gate()[soss];
		_row_values[_sos1Cardinality[soss]] = -1.0;
		set_rowex(_lp, row_no, _sos1Cardinality[soss]+1, _row_values,
				columns_numbers_uk);
		_row_values[_sos1Cardinality[soss]] = +1.0;
		set_constr_type(_lp, row_no, LE);
		set_rh(_lp, row_no, 0.0);

#ifndef _NO_LP_NAMES
		sprintf(_col_or_row_name, "incomp_clique_%d", soss + 1);
		set_row_name(_lp, row_no, _col_or_row_name);
#endif
	}

	return true;
}

bool MathModelColoring::setSOS1()
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

bool MathModelColoring::setLbNoGates(int lb)
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

#ifndef _NO_LP_NAMES
	sprintf(_col_or_row_name, "lower_bound");
	set_row_name(_lp, row_no, _col_or_row_name);
#endif

	return true;
}

bool MathModelColoring::solution(int *&gates) const
{

	gates = new int[_numDwells];
	int Norig_columns, Norig_rows;
	REAL value;
	Norig_columns = get_Norig_columns(_lp);
	Norig_rows = get_Norig_rows(_lp);

	for(int i = 1; i < _uk_start; i++) {
	  value = get_var_primalresult(_lp, Norig_rows + i);
	  if (value >= 0.998) {
			gates[(*_assignment)[i-1].first] =
			(*_assignment)[i-1].second - _numDwells;
		// cout << /*get_col_name(_lp, i+1) << "  " <<*/ _graphs.sets().B()[(*_assignment)[i].first]->dwellNumber() << "; " << _graphs.sets().G()[(*_assignment)[i].second - _numDwells]->gateNumber() << endl;
	  };
	}

	return true;
}

bool MathModelColoring::initialSolution(int *startingSolution)
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
