/*! \file MathModelMaxMinDistance.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */
#include "MathModelMaxMinDistance.h"

using namespace std;
using namespace boost;



MathModelMaxMinDistance::MathModelMaxMinDistance(GraphModel &graphs): MathModel(graphs)
{
	unsigned _numEdgesH = _graphs.numEdgesH();
	unsigned _numEdgesC = _graphs.numEdgesC();
	unsigned _linkConstraints = _graphs.numCompGates();

	_numVars = _numEdgesH + _numEdgesC + 1;
	_numConstraints = _numDwells +     /* Assignment */
	                  _numInterestingCliques +   /* Incompatibility */
	                  _linkConstraints +     /* Y_ij linx to X_ik and X_jk */
	                  _numEdgesC +      /* m */
	                  0;
	_yij_start = _numEdgesH + _xik_start;
	_m_start = _numEdgesC + _yij_start;

	_row_values = new double[_numVars + 1];
	_row_values[0] = 1.0;

	for (int i = 1; i < _numVars + 1; i++) {
		_row_values[i] = 1.0;
	}

	_column_numbers = new int[_numVars + 1];
	_linkrow[0] =  1.;
	_linkrow[1] =  1.;
	_linkrow[2] = -1.;

	_lp = createLP(_numVars, _numConstraints);
	setObjectiveFunction();
	setAssignmentConstraints();
	setIncompatibilityConstraints();
	setLinkConstraints();
	setLinkMConstraints();
	setSOS1();
	set_add_rowmode(_lp, FALSE);
	_presolveOpts =
	    PRESOLVE_ROWS | PRESOLVE_COLS | PRESOLVE_LINDEP |
	    PRESOLVE_SOS |
	    PRESOLVE_REDUCEGCD | PRESOLVE_PROBEFIX | PRESOLVE_PROBEREDUCE |
	    PRESOLVE_COLDOMINATE | PRESOLVE_ROWDOMINATE | PRESOLVE_BOUNDS;
}

MathModelMaxMinDistance::~MathModelMaxMinDistance(void)
{
	delete[]_row_values;
	delete[]_column_numbers;
}

lprec *MathModelMaxMinDistance::createLP(unsigned int numVars,
        unsigned int numConstraints)
{
	if ((_lp = make_lp(numConstraints, numVars)) == NULL) {
		exit(1);
	}

	set_add_rowmode(_lp, TRUE);

	set_lp_name(_lp, "MaxMinDistance");
	set_maxim(_lp);

	//GraphH &H = _graphs.graphH();
	//GraphC &C = _graphs.graphC();

	{
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
			//strcpy(_col_or_row_name, _H_edge_name[*ei].c_str());
			strcpy(_col_or_row_name, _H_edge_name[xik_idx].c_str());
			set_col_name(_lp, variableIndex, _col_or_row_name);
#endif
		}
	}

	{
		graph_traits < GraphC >::edge_iterator ei, edge_end;
		for (tie(ei, edge_end) = edges(_graphs.graphC()); ei != edge_end; ++ei) {
			size_t yij_idx = _C_edge_index[*ei];
			size_t variableIndex = yij_idx + _yij_start;
			// set_binary(_lp, variableIndex, TRUE);
			set_bounds(_lp, variableIndex, 0.0, 1.0);
#ifndef _NO_LP_NAMES
			strcpy(_col_or_row_name, _C_edge_name[yij_idx].c_str());
			set_col_name(_lp, variableIndex, _col_or_row_name);
#endif
		}
	}

#ifndef _NO_LP_NAMES
	strcpy(_col_or_row_name, "m");
	set_col_name(_lp, _m_start, _col_or_row_name);
#endif

	return _lp;
}

bool MathModelMaxMinDistance::setObjectiveFunction()
{
	if (set_obj(_lp, _m_start, 1.0) == FALSE) {
		return FALSE;
	}

	_currentRow = 0;
	set_row_name(_lp, _currentRow, "MaxMinDistance");
	_currentRow++;

	return true;
}

bool MathModelMaxMinDistance::setAssignmentConstraints()
{
	GraphH &H = _graphs.graphH();

	/* for each dwell */
	for (unsigned int i = 0; i < _numDwells; i++) {
#ifndef _NO_LP_NAMES
		sprintf(_col_or_row_name, "dwell_%d", i + 1);
		set_row_name(_lp, _currentRow, _col_or_row_name);
#endif

		graph_traits < GraphH >::out_edge_iterator out_i, out_end;
		unsigned count = 0;
		for (tie(out_i, out_end) = out_edges(i, H); out_i != out_end;
		        ++out_i) {
			_column_numbers[count++] = _H_edge_index[*out_i] + _xik_start;
		}

		set_rowex(_lp, _currentRow, count, _row_values, _column_numbers);
		set_constr_type(_lp, _currentRow, EQ);
		set_rh(_lp, _currentRow++, 1.0);
		add_SOS(_lp, NULL, 1, 1, count,  _column_numbers, NULL);
	}

	return true;
}

bool MathModelMaxMinDistance::setIncompatibilityConstraints()
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

bool MathModelMaxMinDistance::setLinkConstraints()
{

	Buses &B = _graphs.B();
	//Gates &G = _graphs.G();
	GraphC &C = _graphs.graphC();
	//GraphH &H = _graphs.graphH();
	int fictitious_k = 0;

	graph_traits < GraphC >::edge_iterator ei, ei_end;
	VertexDescriptorC i, j;

	/* 1: \forall (i, j) \in F */
	for (tie(ei, ei_end) = edges(C); ei != ei_end; ++ei) {
		i = source(*ei, C);
		j = target(*ei, C);

		//if (j <= i) {
		//	continue;
		//}

#ifndef _NO_LP_NAMES
		size_t iIndex = _C_vertex_index[i];
		size_t jIndex = _C_vertex_index[j];
#endif
		size_t y_ijIndex = _C_edge_index[*ei];

		for (std::vector<std::pair<size_t, size_t> > ::const_iterator x_ijk = _graphs.compGates()[y_ijIndex]->begin();
		        x_ijk != _graphs.compGates()[y_ijIndex]->end(); x_ijk++) {
			size_t x_ik = x_ijk->first;
			size_t x_jk = x_ijk->second;

			_linkcolno[0] = x_ik + _xik_start;
			_linkcolno[1] = x_jk + _xik_start;
			_linkcolno[2] = y_ijIndex + _yij_start;

			set_rowex(_lp, _currentRow, 3, _linkrow, _linkcolno);

			/*VertexDescriptor k =  target(x_ik, H);*/
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

bool MathModelMaxMinDistance::setLinkMConstraints()
{

	Buses &B = _graphs.B();
	GraphC &C = _graphs.graphC();
	boost::graph_traits < GraphC >::edge_iterator ei, ei_end;
	VertexDescriptorC i, j;

	double _bigM = 0;
	double lb = 10E5;
	for (tie(ei, ei_end) = edges(_graphs.graphC()); ei != ei_end; ++ei) {
		size_t yij_idx = _C_edge_index[*ei];
		double cost = _C_edge_weight[yij_idx];

		if (_bigM < cost) {
			_bigM = cost;
		}
		if (cost < lb)
			lb = cost;
	}

	// _linkrow[0] =  1.;
	// _column_numbers[0] = _m_start;

	/* 1: \forall (i, j) \in F */
	for (tie(ei, ei_end) = edges(C); ei != ei_end; ++ei) {
		i = source(*ei, C);
		j = target(*ei, C);
		size_t yij_idx = _C_edge_index[*ei];

		/*if (j <= i) {
			continue;
		}*/

#ifdef _NO_LP_NAMES
		int iIndex = _C_vertex_index[i];
		int jIndex = _C_vertex_index[j];
		sprintf(_col_or_row_name, "m_y_%d_%d", B[iIndex]->dwellNumber(),
		        B[jIndex]->dwellNumber());
		set_row_name(_lp, _currentRow, _col_or_row_name);
#endif
		_column_numbers[0] = _m_start;
		_column_numbers[1] = yij_idx + _yij_start;
		_linkrow[0] =  1.;
		_linkrow[1] =  _bigM - _C_edge_weight[yij_idx];
		set_rowex(_lp, _currentRow, 2, _linkrow, _column_numbers);
		set_constr_type(_lp, _currentRow, LE);
		set_rh(_lp, _currentRow, _bigM);
		_currentRow++;
	}

	_linkrow[0] =  1.;
	_linkrow[1] =  1.;

	set_bounds(_lp, _m_start, lb, _bigM);

	return true;
}

bool MathModelMaxMinDistance::setSOS1()
{

	for (int soss = 0; soss < _numInterestingCliques; soss++) {
		sprintf(_col_or_row_name, "SOS_%d", soss + 1);
		add_SOS(_lp, _col_or_row_name, 1, 1, _sos1Cardinality[soss],
		        _colno[soss], NULL);
	}

	return true;
}

bool MathModelMaxMinDistance::solution(int *&gates)  const
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
	vector < vector <size_t> > gate_dwell (_objectiveFunction);
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

bool MathModelMaxMinDistance::initialSolution(int *startingSolution)
{
	double *varValues = new double[1 + _numVars];
	int *basisvector = new int[1 + _numVars + _numConstraints];
	double m = 1E10;

	for (int i = 0; i < 1 + _numVars; i++)
		varValues[i] = 0.0;

	// cerr << ">> Inizio a ricostruire la soluzione iniziale <<" << endl;

	for (unsigned int dwell = 0; dwell < _graphs.numDwells(); dwell++) {
		size_t i = _graphs.B()[dwell]->vertexIndex();
		unsigned int gate = startingSolution[dwell];
		size_t k = _graphs.G()[gate]->vertexIndex();
		/* set x_ik = 1 */
		assert(edge(i, k, _H).second);
		size_t xik_idx = _H_edge_index[edge(i, k, _H).first];
		size_t x_ik = xik_idx + _xik_start;
		varValues[x_ik] = 1.;
		/* cerca fra le soste precedenti quelle associate allo stesso gate */
		for (unsigned int otherDwell = 0; otherDwell < dwell; otherDwell++) {
			if (startingSolution[otherDwell] == gate) {
				size_t j = _graphs.B()[otherDwell]->vertexIndex();
				/* setta y_ij */
				assert(edge(i, j, _C).second);
				size_t yij_idx = _C_edge_index[edge(i, j, _C).first];
				size_t y_ij = yij_idx + _yij_start;
				varValues[y_ij] = 1.;
				if (_C_edge_weight[yij_idx] < m)
					m = _C_edge_weight[yij_idx];
				// break;
			}
		}
	}
	varValues[_m_start] = m;

	//cerr << ">> Soluzione iniziale ricostruita <<" << endl;

	/*for (int i = 0; i < _numVars + 1; i++)
		if (abs(varValues[i]) > 1E-10 || i == _m_start)
				cerr << get_col_name(_lp, i) << " = "  << varValues[i] << endl;*/

	bool status;
	if ( guess_basis(_lp, varValues, basisvector) == TRUE) {
		//cerr << ">> LP_SOLVE ha riconosciuto una base iniziale! <<" << endl;
		if (set_basis(_lp, basisvector, TRUE) == TRUE) {
			//cerr << ">> >> Base settata!!!!!!! << <<" << endl;
			write_basis(_lp, "modelMaxMinDistance.bas");
			status = true;
		} else
			status = false;
	} else {
		//cerr << ">> PORCA!!! PORCA!!! PORCA!!! PORCA!!! PORCA!!! <<" << endl;
		status = false;
	}

	delete []varValues;
	delete []basisvector;

	return status;
}

