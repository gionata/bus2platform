/*! \file MathModelMaxTotalDistance.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */
#include "MathModelMaxTotalDistance.h"

using namespace std;
using namespace boost;

MathModelMaxTotalDistance::MathModelMaxTotalDistance(GraphModel &graphs): 
        MathModel(graphs)
{
	_solved = false;
	_col_or_row_name = new char[100];
	_numVars = _graphs.numEdgesH() + _graphs.numEdgesC();
	_numConstraints = _numDwells +     /* Assignment */
				   _graphs.numInterestingCliques() +    /* Incompatibility */
				   _graphs.numEdgesC() + /* Y_ij linx to X_ik and X_jk */
				   _graphs.numEdgesC() + /* Y_ij linx to X_ik and X_jk */
				   _graphs.numEdgesC();  /* Y_ij linx to X_ik and X_jk */
	_xik_start = 1;
	_yij_start = _graphs.numEdgesH() + 1;
	
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
	setSOS1();
	write_lp(_lp, "modelMaxTotalDistance.lp");
	set_XLI(_lp, "xli_CPLEX");
	write_XLI(_lp, "modelMaxTotalDistance_CPLEX.lp", 0, FALSE);
}

MathModelMaxTotalDistance::~MathModelMaxTotalDistance(void)
{
	delete[]_row_values;
}

lprec *MathModelMaxTotalDistance::createLP(unsigned int numVars,
		unsigned int numConstraints)
{
	if ((_lp = make_lp(numConstraints, numVars)) == NULL) {
		exit(1);
	}

	set_lp_name(_lp, "MaxTotalDistance");
	set_maxim(_lp);

	graph_traits < GraphH >::edge_iterator eiH, edge_endH;

	for (tie(eiH, edge_endH) = edges(_graphs.graphH()); eiH != edge_endH; ++eiH) {
		size_t xik_idx = _H_edge_index[*eiH];
		int variableIndex = xik_idx + _xik_start;
		(*_assignment)[xik_idx].first =
			source(*eiH, _graphs.graphH());
		(*_assignment)[xik_idx].second =
			target(*eiH, _graphs.graphH());
		set_binary(_lp, variableIndex, TRUE);
		//strcpy(_col_or_row_name, _H_edge_name[*ei].c_str());
		strcpy(_col_or_row_name, _H_edge_name[xik_idx].c_str());
		set_col_name(_lp, variableIndex, _col_or_row_name);
	}

	graph_traits < GraphC >::edge_iterator eiC, edge_endC;
	for (tie(eiC, edge_endC) = edges(_graphs.graphC()); eiC != edge_endC; ++eiC) {
		size_t yij_idx = _C_edge_index[*eiC];
		int variableIndex = yij_idx + _yij_start;
		// set_binary(_lp, variableIndex, TRUE);
		set_bounds(_lp, variableIndex, 0.0, 1.0);
		strcpy(_col_or_row_name, _C_edge_name[yij_idx].c_str());
		set_col_name(_lp, variableIndex, _col_or_row_name);
	}

	return _lp;
}

bool MathModelMaxTotalDistance::setObjectiveFunction()
{
	graph_traits < GraphC >::edge_iterator eiC, edge_endC;

	for (tie(eiC, edge_endC) = edges(_graphs.graphC()); eiC != edge_endC; ++eiC) {
		size_t yij_idx = _C_edge_index[*eiC];
		int variableIndex = yij_idx + _yij_start;

		if (set_obj(_lp, variableIndex, _C_edge_weight[yij_idx]) == FALSE) {
			return FALSE;
		}
	}

	_currentRow = 0;
	set_row_name(_lp, _currentRow, "TotalDistance");
	_currentRow++;

	return TRUE;
}

bool MathModelMaxTotalDistance::setAssignmentConstraints()
{

	/* for each dwell */
	for (unsigned int i = 0; i < _numDwells; i++) {
		sprintf(_col_or_row_name, "dwell_%d", i + 1);
		set_row_name(_lp, _currentRow, _col_or_row_name);
		graph_traits < GraphH >::out_edge_iterator out_iH, out_endH;

		for (tie(out_iH, out_endH) = out_edges(i, _graphs.graphH());
				out_iH != out_endH; ++out_iH) {
			set_mat(_lp, _currentRow,
				   _H_edge_index[*out_iH] + _xik_start, 1.0);
		}

		set_constr_type(_lp, _currentRow, EQ);
		set_rh(_lp, _currentRow++, 1.0);
	}

	return TRUE;
}

bool MathModelMaxTotalDistance::setIncompatibilityConstraints()
{

	for (int soss = 0; soss < _numInterestingCliques; soss++) {
		sprintf(_col_or_row_name, "incomp_clique_%d", soss + 1);
		set_rowex(_lp, _currentRow, _sos1Cardinality[soss], _row_values,
				_colno[soss]);
		set_constr_type(_lp, _currentRow, LE);
		set_rh(_lp, _currentRow, 1.0);
		set_row_name(_lp, _currentRow++, _col_or_row_name);
	}

	return true;
}

bool MathModelMaxTotalDistance::setLinkConstraints()
{

	Buses &B = _graphs.B();
	GraphC &C = _graphs.graphC();
	//GraphH &H = _graphs.graphH();
	int currentRow2;
	int currentRow3;
	int *colno[3];

	for (int i = 0; i < 3; i++) {
		colno[i] = new int[_graphs.numEdgesH()];
	}

	graph_traits < GraphC >::edge_iterator eiC, ei_endC;
	VertexDescriptorC i, j;

	/* 1: \forall (i, j) \in F */
	for (tie(eiC, ei_endC) = edges(C); eiC != ei_endC; ++eiC) {
		i = source(*eiC, C);
		j = target(*eiC, C);

		if (j <= i) {
			continue;
		}

		int iIndex = _C_vertex_index[i];
		int jIndex = _C_vertex_index[j];
		currentRow2 = _currentRow + _graphs.numEdgesC();
		currentRow3 = currentRow2 + _graphs.numEdgesC();
		sprintf(_col_or_row_name, "l1_y_%d_%d",
			   B[iIndex]->dwellNumber(), B[jIndex]->dwellNumber());
		set_row_name(_lp, _currentRow, _col_or_row_name);
		//sprintf(_col_or_row_name, "l2_y_%d_%d", B[iIndex]->dwellNumber(),  B[jIndex]->dwellNumber());
		_col_or_row_name[1] = '2';
		set_row_name(_lp, currentRow2, _col_or_row_name);
		//sprintf(_col_or_row_name, "l3_y_%d_%d", B[iIndex]->dwellNumber(),  B[jIndex]->dwellNumber());
		_col_or_row_name[1] = '3';
		set_row_name(_lp, currentRow3, _col_or_row_name);

/*
		std::set < int >*&linkXiYij =
			_graphs.linkXiYij()[_C_edge_index[*ei]];
		std::set < int >*&linkXjYij =
			_graphs.linkXjYij()[_C_edge_index[*ei]];
*/

		/*
		   cout << "Generating link constraints of (" << B[iIndex]->id()
		   << ", " << B[jIndex]->id() << ") in F. [" << _C_edge_index[*ei]
		   << "/" << _graphs.numEdgesC() << "]" << endl;
		 */
/*
		int countI = 0;

		for (set < int >::const_iterator x_ik = linkXiYij->begin();
				x_ik != linkXiYij->end(); x_ik++) {
			colno[0][countI] = *x_ik + _xik_start;
			colno[2][countI++] = *x_ik + _xik_start;
		}

		int countJ = 0;

		for (set < int >::const_iterator x_jk = linkXjYij->begin();
				x_jk != linkXjYij->end(); x_jk++) {
			colno[1][countJ] = *x_jk + _xik_start;
			colno[2][countI + countJ++] = *x_jk + _xik_start;
		}

		set_rowex(_lp, _currentRow, countI, _row_values, colno[0]);
		set_rowex(_lp, currentRow2, countJ, _row_values, colno[1]);
		set_rowex(_lp, currentRow3, countI + countJ, _row_values,
				colno[2]);

		set_mat(_lp, _currentRow, _H_edge_index[*ei] + _yij_start,
			   -1.0);
		set_constr_type(_lp, _currentRow, GE);
		set_rh(_lp, _currentRow, 0.0);

		set_mat(_lp, currentRow2, _H_edge_index[*ei] + _yij_start,
			   -1.0);
		set_constr_type(_lp, currentRow2, GE);
		set_rh(_lp, currentRow2, 0.0);

		set_mat(_lp, currentRow3, _H_edge_index[*ei] + _yij_start,
			   -1.0);
		set_constr_type(_lp, currentRow3, LE);
		set_rh(_lp, currentRow3, 1.0);
		_currentRow++;
		*/
	}

	_currentRow += 2 * _graphs.numEdgesC();

	for (int i = 0; i < 3; i++) {
		delete[]colno[i];
	}

	return true;
}

bool MathModelMaxTotalDistance::setSOS1()
{

	for (int soss = 0; soss < _numInterestingCliques; soss++) {
		sprintf(_col_or_row_name, "SOS_%d", soss + 1);
		add_SOS(_lp, _col_or_row_name, 1, 1, _sos1Cardinality[soss],
			   _colno[soss], NULL);
	}

	return TRUE;
}

bool MathModelMaxTotalDistance::solution(int *&gates)  const
{
	double *sol = new double[_numVars];
	gates = new int[_numDwells];

	if (get_variables(_lp, sol) == FALSE) {
		return false;
	}

	for (int i = 0; i < _yij_start - 1; i++)
		if (sol[i] >= 0.998) {
			gates[(*_assignment)[i].first] =
				(*_assignment)[i].second - _numDwells;
			// cout << /*get_col_name(_lp, i+1) << "  " <<*/ _graphs.sets().B()[(*_assignment)[i].first]->dwellNumber() << "; " << _graphs.sets().G()[(*_assignment)[i].second - _numDwells]->gateNumber() << endl;
		}

	delete[]sol;
	return true;
}

/*
unsigned char MathModelMaxTotalDistance::writeModelLP_solve(char *filename)
{
	return write_lp(_lp, filename);
}

unsigned char MathModelMaxTotalDistance::writeModelCPLEX(char *filename)
{
	set_XLI(_lp, "xli_CPLEX");
	return write_XLI(_lp, filename, 0, FALSE);
}
*/

