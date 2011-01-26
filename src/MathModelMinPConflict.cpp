/*! \file MathModelMinPConflict.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */
#include "MathModelMinPConflict.h"
#include <cmath>

using namespace std;
using namespace boost;

// #define LINEAR_FUNCT

MathModelMinPConflict::MathModelMinPConflict(GraphModel &graphs): MathModel(graphs), _D(0.0), _d(100000.0), _sr()
{
	unsigned _numEdgesH = _graphs.numEdgesH();
	unsigned _numEdgesC = _graphs.numEdgesC();
	unsigned _linkConstraints = _graphs.numCompGates();

	_presolveOpts =
	    PRESOLVE_ROWS | PRESOLVE_COLS | PRESOLVE_LINDEP |
	    PRESOLVE_SOS |
	    PRESOLVE_REDUCEGCD | PRESOLVE_PROBEFIX | PRESOLVE_PROBEREDUCE |
	    PRESOLVE_COLDOMINATE | PRESOLVE_ROWDOMINATE | PRESOLVE_BOUNDS;

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

	_column_numbers = new int[_numVars + 1];

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
	// setSOS1();
	end = clock();
	mstime = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
	_sr.add(SolutionInformation("sos1LPtime (ms)", "Time to construct the SOS1 constraints", int_long, &(mstime)));

	set_add_rowmode(_lp, FALSE);

	//cout << "constraints, secondo previsione: " << _numConstraints << endl;
	//cout << "constraints, secondo codice:     " << get_Nrows(_lp) << endl;
}

MathModelMinPConflict::~MathModelMinPConflict(void)
{
	// _sr.print(cout);
	delete[]_row_values;
	delete[]_column_numbers;
}

lprec *MathModelMinPConflict::createLP(unsigned int numVars, unsigned int numConstraints)
{
	if ((_lp = make_lp(numConstraints, numVars)) == NULL) {
		exit(1);
	}

	set_add_rowmode(_lp, TRUE);

#ifndef _NO_LP_NAMES
	set_lp_name(_lp, "MinPConflict");
#endif
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

	boost::graph_traits < GraphC >::edge_iterator eiC, ei_endC;

	double cost [_graphs.numEdgesC()];
	for (tie(eiC, ei_endC) = edges(_graphs.graphC()); eiC != ei_endC; ++eiC) {
		size_t yij_idx = _C_edge_index[*eiC];

#ifdef LINEAR_FUNCT
		cost[yij_idx] =_C_edge_weight[yij_idx];
#else
		double conflict = 0.0;
		if (_C_edge_weight[yij_idx] <= 0)
			conflict = 1.0;
		else if (_C_edge_weight[yij_idx] <= 30.0)
			conflict = exp(-0.05*_C_edge_weight[yij_idx]);
		else
			conflict =  0.0;
		cost[yij_idx] = 1.0 - conflict;
#endif

		if (_D < cost[yij_idx]) {
			_D = cost[yij_idx];
		}

		if (_d > cost[yij_idx]) {
			_d = cost[yij_idx];
		}
	}

	double den =
#ifdef LINEAR_FUNCT
	    _graphs.numEdgesC() *
#endif
	    (_D - _d);

	for (tie(eiC, ei_endC) = edges(_graphs.graphC()); eiC != ei_endC; ++eiC) {
		size_t yij_idx = _C_edge_index[*eiC];

		int variableIndex = yij_idx + _yij_start;
//std::cout << "edge: " << yij_idx << " weight: " << _C_edge_weight[yij_idx] << " cost: " << (_D - cost[yij_idx]) / den << endl;

		if (_D - cost[yij_idx] > 0)
			if (set_obj(_lp, variableIndex, (_D - cost[yij_idx]) / den) ==
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
		unsigned count = 0;
		for (tie(out_iH, out_endH) = out_edges(i, _graphs.graphH());
		        out_iH != out_endH; ++out_iH) {
			_column_numbers[count++] = _H_edge_index[*out_iH] + _xik_start;
		}

		set_rowex(_lp, _currentRow, count, _row_values, _column_numbers);
		set_constr_type(_lp, _currentRow, EQ);
		set_rh(_lp, _currentRow++, 1.0);
		add_SOS(_lp, NULL, 1, 1, count,  _column_numbers, NULL);
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
		add_SOS(_lp, NULL, 1, 1, _sos1Cardinality[soss],
		        _colno[soss], NULL);
	}

	return true;
}

bool MathModelMinPConflict::setLinkConstraints()
{

	Buses &B = _graphs.B();
	GraphC &C = _graphs.graphC();
	//GraphH &H = _graphs.graphH();
	int fictitious_k = 0;

	_row_values[2] = -1.;

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
		        x_ijk != _graphs.compGates()[y_ijIndex]->end(); x_ijk++) {
			size_t x_ik = x_ijk->first;
			size_t x_jk = x_ijk->second;
			_column_numbers[0] = x_ik + _xik_start;
			_column_numbers[1] = x_jk + _xik_start;
			_column_numbers[2] = y_ijIndex + _yij_start;

			set_rowex(_lp, _currentRow, 3, _row_values, _column_numbers);

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

	_row_values[2] = 1.;

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
	gates = new int[_numDwells];
	if (_presolveOpts != PRESOLVE_NONE) {
		int Norig_columns, Norig_rows;
		REAL value;
		Norig_columns = get_Norig_columns(_lp);
		Norig_rows = get_Norig_rows(_lp);

		for(int i = 1; i < _yij_start; i++) {
			value = get_var_primalresult(_lp, Norig_rows + i);
			if (value >= 0.998) {
				gates[(*_assignment)[i-1].first] =
				    (*_assignment)[i-1].second - _numDwells;
				// cout << /*get_col_name(_lp, i+1) << "  " <<*/ _graphs.sets().B()[(*_assignment)[i].first]->dwellNumber() << "; " << _graphs.sets().G()[(*_assignment)[i].second - _numDwells]->gateNumber() << endl;
			};
		}
	} else {
		double *sol = new double[_numVars];
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
	}

	//////////////   VERIFICA AMMISSIBILITA   //////////////
	//                                                    //
	//     verifica di ammissibilita' della soluzione     //
	//                                                    //
	////////////////////////////////////////////////////////
	vector < vector <size_t> > gate_dwell (_graphs.G().size());
	bool used[_graphs.G().size()];
	for (int i = 0; i < _graphs.G().size(); i++) {
		used[i] = false;
	}
	// setta il vettore gate_dwell
	for (int d = 0; d < _numDwells; d++) {
		size_t platform = gates[d];
		if (!used[platform]) {
			used[platform] = true;
			// gate_dwell[platform]= new vector<size_t>();
		}
		gate_dwell[platform].push_back(d);
	}
	// controlla, per ogni piattaforma
	for (int p = 0; p < _graphs.G().size(); p++) {
		if (!used[p])
			continue;
		// che ogni sosta
		for (vector<size_t>::const_iterator i = gate_dwell[p].begin(); i != gate_dwell[p].end(); i++) {
			// sia assegnabile alla piattaforma
			if (! _graphs.B()[*i]->compatible(_graphs.G()[p])) {
				cerr << "sosta " << _graphs.B()[*i]->dwellNumber() << " non compatibile con piattaforma "<< p + 1 << endl;
				exit(1);
			}
			// non interferisca con nessuna delle soste successive
			if (*i != gate_dwell[p].back()) {
				vector<size_t>::const_iterator j = i;
				j++;
				for (; j != gate_dwell[p].end(); j++) {
					if (_graphs.B()[*i]->occupacyPeriod().intersects(_graphs.B()[*j]->occupacyPeriod())) {
						cerr << "piattaforma " << p + 1 << ", sosta " <<
						     _graphs.B()[*i]->dwellNumber() << " interferisce con " << _graphs.B()[*j]->dwellNumber() << endl;
						exit(1);
					}
				}
			}
		}
	}

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
			cerr << ">> >> Base settata!!!!!!! << <<" << endl;
			write_basis(_lp, "modelMinPConflict_initial.bas");
			status = true;
		}
	} else {
		cerr << "\a>> Non riesco a fornire soluzione iniziale <<" << endl;
		status = false;
	}

	delete []varValues;
	delete []basisvector;

	return status;
}
