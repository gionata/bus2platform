/*! \file MathModelMinPConflict.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */
#include "MathModelMinPConflict.h"

using namespace std;
using namespace boost;

MathModelMinPConflict::MathModelMinPConflict(GraphModel &graphs): MathModel(graphs), _D(0.0), _d(100000.0), _sr()
{
	unsigned _numEdgesH = _graphs.numEdgesH();
	unsigned _numEdgesC = _graphs.numEdgesC();
	unsigned _linkConstraints = _graphs.numCompGates();

	clock_t begin = clock();

	_numVars = _numEdgesH + _numEdgesC + 1;
	_numConstraints = _numDwells +     /* Assignment */
				   _numInterestingCliques +   /* Incompatibility */
				   _linkConstraints +    /* Y_ij linx to X_ik and X_jk */
				   0;
	// _xik_start = 1;
	_yij_start = _numEdgesH + _xik_start;

	_row_values = new double[_numVars + 1];
	_row_values[0] = 1.0;

	for (int i = 1; i < _numVars + 1; i++) {
		_row_values[i] = 1.0;
	}

	_lp = createLP(_numVars, _numConstraints);
	clock_t end = clock();
	long int mstime = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
	_sr.add(SolutionInformation("emptyLPtime (ms)", "Time to construct the empty LP", int_long, &(mstime)));

	begin = clock();
	setObjectiveFunction();
	end = clock();
	mstime = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
	_sr.add(SolutionInformation("ofLPtime (ms)", "Time to construct the objective function", int_long, &(mstime)));

	begin = clock();
	setAssignmentConstraints();
	end = clock();
	mstime = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
	_sr.add(SolutionInformation("assignLPtime (ms)", "Time to construct the assignment constraints", int_long, &(mstime)));

	begin = clock();
	setIncompatibilityConstraints();
	end = clock();
	mstime = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
	_sr.add(SolutionInformation("incompLPtime (ms)", "Time to construct the incompatibility constraints", int_long, &(mstime)));

	begin = clock();
	setLinkConstraints();
	end = clock();
	mstime = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
	_sr.add(SolutionInformation("linkLPtime (ms)", "Time to construct the link constraints", int_long, &(mstime)));

	begin = clock();
	setSOS1();
	end = clock();
	mstime = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
	_sr.add(SolutionInformation("sos1LPtime (ms)", "Time to construct the SOS1 constraints", int_long, &(mstime)));


	//cout << "constraints, secondo previsione: " << _numConstraints << endl;
	//cout << "constraints, secondo codice:     " << get_Nrows(_lp) << endl;

//	write_lp(_lp, "modelMinPConflict.lp");
//	set_XLI(_lp, "xli_CPLEX");
//	write_XLI(_lp, "modelMinPConflict_CPLEX.lp", 0, FALSE);
}

MathModelMinPConflict::~MathModelMinPConflict(void)
{
	// _sr.print(cout);
	delete[]_row_values;
}

lprec *MathModelMinPConflict::createLP(unsigned int numVars,
							    unsigned int numConstraints)
{
	if ((_lp = make_lp(numConstraints, numVars)) == NULL) {
		exit(1);
	}

	set_lp_name(_lp, "MinPConflict");
	set_minim(_lp);

	graph_traits < GraphH >::edge_iterator eiH, edge_endH;
	for (tie(eiH, edge_endH) = edges(_graphs.graphH()); eiH != edge_endH; ++eiH) {
		size_t xik_idx = _H_edge_index[*eiH];
		int variableIndex = xik_idx + _xik_start;
		(*_assignment)[xik_idx].first =
			source(*eiH, _graphs.graphH());
		(*_assignment)[xik_idx].second =
			target(*eiH, _graphs.graphH());
		set_binary(_lp, variableIndex, TRUE);
#ifndef _NO_LP_NAMES
		// strcpy(_col_or_row_name, _H_edge_name[*eiH].c_str());
		strcpy(_col_or_row_name, _H_edge_name[xik_idx].c_str());
		set_col_name(_lp, variableIndex, _col_or_row_name);
#endif
	}

	graph_traits < GraphC >::edge_iterator eiC, edge_endC;
	for (tie(eiC, edge_endC) = edges(_graphs.graphC()); eiC != edge_endC; ++eiC) {
		size_t yij_idx = _C_edge_index[*eiC];
		int variableIndex = yij_idx + _yij_start;
		set_binary(_lp, variableIndex, TRUE);
		set_bounds(_lp, variableIndex, 0.0, 1.0);
#ifndef _NO_LP_NAMES
		strcpy(_col_or_row_name, _C_edge_name[yij_idx].c_str());
		set_col_name(_lp, variableIndex, _col_or_row_name);
#endif
	}

	return _lp;
}

bool MathModelMinPConflict::setObjectiveFunction()
{

	// D = max {c_ij}
	boost::graph_traits < GraphC >::edge_iterator eiC, ei_endC;

	for (tie(eiC, ei_endC) = edges(_graphs.graphC()); eiC != ei_endC; ++eiC) {
		size_t yij_idx = _C_edge_index[*eiC];
		double cost = _C_edge_weight[yij_idx];

		if (_D < cost) {
			_D = cost;
		}

		if (_d > cost) {
			_d = cost;
		}
	}

	double den = _graphs.numEdgesC() * (_D - _d);

	for (tie(eiC, ei_endC) = edges(_graphs.graphC()); eiC != ei_endC; ++eiC) {
		size_t yij_idx = _C_edge_index[*eiC];
		double cost = _C_edge_weight[yij_idx];
		int variableIndex = yij_idx + _yij_start;

		if (_D - cost > 0)
			if (set_obj(_lp, variableIndex, (_D - cost) / den) ==
					FALSE) {
				return false;
			}
	}

	_currentRow = 0;
#ifndef _NO_LP_NAMES
	set_row_name(_lp, _currentRow, "ConflictProbability");
#endif
	_currentRow++;

	return true;
}

bool MathModelMinPConflict::setAssignmentConstraints()
{

	/* for each dwell */
	for (unsigned int i = 0; i < _numDwells; i++) {
#ifndef _NO_LP_NAMES
		sprintf(_col_or_row_name, "dwell_%d", i + 1);
		set_row_name(_lp, _currentRow, _col_or_row_name);
#endif
		graph_traits < GraphH >::out_edge_iterator out_iH, out_endH;

		for (tie(out_iH, out_endH) = out_edges(i, _graphs.graphH());
				out_iH != out_endH; ++out_iH) {
			set_mat(_lp, _currentRow,
				   _H_edge_index[*out_iH] + _xik_start, 1.0);
		}

		set_constr_type(_lp, _currentRow, EQ);
		set_rh(_lp, _currentRow++, 1.0);
	}

	return true;
}

bool MathModelMinPConflict::setIncompatibilityConstraints()
{

	for (int soss = 0; soss < _numInterestingCliques; soss++) {

		set_rowex(_lp, _currentRow, _sos1Cardinality[soss], _row_values,
				_colno[soss]);
		set_constr_type(_lp, _currentRow, LE);
		set_rh(_lp, _currentRow, 1.0);
#ifndef _NO_LP_NAMES
		sprintf(_col_or_row_name, "incomp_clique_%d", soss + 1);
		set_row_name(_lp, _currentRow++, _col_or_row_name);
#endif
	}

	return true;
}

bool MathModelMinPConflict::setLinkConstraints()
{

	Buses &B = _graphs.B();
	GraphC &C = _graphs.graphC();
	//GraphH &H = _graphs.graphH();
	int fictitious_k = 0;

	graph_traits < GraphC >::edge_iterator eiC, ei_endC;
	VertexDescriptorC i, j;

	/* 1: \forall (i, j) \in F */
	for (tie(eiC, ei_endC) = edges(C); eiC != ei_endC; ++eiC) {
		i = source(*eiC, C);
		j = target(*eiC, C);

//		if (j >= i) {
//			continue;
//		}
		
#ifndef _NO_LP_NAMES
		int iIndex = _C_vertex_index[i];
		int jIndex = _C_vertex_index[j];
#endif
		int y_ijIndex = _C_edge_index[*eiC];

		for (std::vector<std::pair<size_t, size_t> > ::const_iterator x_ijk = _graphs.compGates()[y_ijIndex]->begin();
			x_ijk != _graphs.compGates()[y_ijIndex]->end(); x_ijk++)
		{
			size_t x_ik = x_ijk->first;
			size_t x_jk = x_ijk->second;
			set_mat(_lp, _currentRow, x_ik + _xik_start,
				   1);
			set_mat(_lp, _currentRow, x_jk + _xik_start,
				   1);
			set_mat(_lp, _currentRow, y_ijIndex + _yij_start,
				   -1);
			/*int k =  (x_ik, H)
			sprintf(_col_or_row_name, "link_%d_%d_%d", B[iIndex]->dwellNumber(),
			   B[jIndex]->dwellNumber(), k);
			set_row_name(_lp, _currentRow, _col_or_row_name);*/
#ifndef _NO_LP_NAMES
			sprintf(_col_or_row_name, "link_%d_%d_%d", B[iIndex]->dwellNumber(),
						   B[jIndex]->dwellNumber(), fictitious_k++ % _graphs.numPlatforms() + 1);
			set_row_name(_lp, _currentRow, _col_or_row_name);
#endif
			set_constr_type(_lp, _currentRow, LE);
			set_rh(_lp, _currentRow, 1);
			_currentRow++;
		}
	}

	return true;
}

bool MathModelMinPConflict::setSOS1()
{

	for (int soss = 0; soss < _numInterestingCliques; soss++) {
		sprintf(_col_or_row_name, "SOS_%d", soss + 1);
		add_SOS(_lp, _col_or_row_name, 1, 1, _sos1Cardinality[soss],
			   _colno[soss], NULL);
	}

	return true;
}

bool MathModelMinPConflict::solution(int *&gates)  const
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

bool MathModelMinPConflict::initialSolution(int *startingSolution)
{
	double *varValues = new double[1 + _numVars];
	int *basisvector = new int[1 + _numVars + _numConstraints];
	double conflict = 0.0;
	const double den = _graphs.numEdgesC() * (_D - _d);

	for (int i = 0; i < 1 + _numVars; i++)
		varValues[i] = 0.0;

	// cerr << ">> Inizio a ricostruire la soluzione iniziale <<" << endl;

	for (unsigned int dwell = 0; dwell < _graphs.numDwells(); dwell++) {
		size_t i = _graphs.B()[dwell]->vertexIndex();
		unsigned int gate = startingSolution[dwell];
		size_t k = _graphs.G()[gate]->vertexIndex();
		/* set x_ik = 1 */
		// assert(edge(i, k, _H).second);
		size_t x_ik = _H_edge_index[edge(i, k, _H).first] + _xik_start;
		varValues[x_ik] = 1.;
		/* cerca fra le soste precedenti quelle associate allo stesso gate */
		for (unsigned int otherDwell = 0; otherDwell < dwell; otherDwell++) {
			if (startingSolution[otherDwell] == gate) {
				size_t j = _graphs.B()[otherDwell]->vertexIndex();
				/* setta y_ij */
				// assert(edge(i, j, _C).second);
				size_t y_ij = _C_edge_index[edge(i, j, _C).first] + _yij_start;
				varValues[y_ij] = 1.;
			}
		}
	}

	// cerr << ">> Soluzione iniziale ricostruita <<" << endl;

	/*
	for (int i = 0; i < _numVars + 1; i++)
		if (abs(varValues[i]) > 1E-10)
				cerr << get_col_name(_lp, i) << " = "  << varValues[i] << endl;
	*/

	bool status;
	if ( guess_basis(_lp, varValues, basisvector) == TRUE) {
		// cerr << "\a>> LP_SOLVE ha riconosciuto una base iniziale! <<" << endl;
		if (set_basis(_lp, basisvector, TRUE) == TRUE) {
			//cerr << ">> >> Base settata!!!!!!! << <<" << endl;
			//write_basis(_lp, "modelMinPConflict.bas");
			status = true;
		}
	} else {
		//cerr << "\a>> Non riesco a fornire soluzione iniziale <<" << endl;
		status = false;
	}

	delete []varValues;
	delete []basisvector;

	return status;
}
