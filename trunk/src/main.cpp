/*! \file main.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "gap.h"
#include "Bus.h"
#include "Gate.h"
#include "SetModel.h"
#include "ListColoringTreeSearch.h"
#include "GraphModel.h"
#include "MathModelBP.h"
#include "MathModelBPsingle.h"
#include "MathModelColoring.h"
#include "MathModelMaxTotalDistance.h"
#include "MathModelMaxMinDistance.h"
#include "MathModelMinPConflict.h"
#include "IntervalPackingFinishFirst.h"
#include "IntervalPackingFirstFit.h"
#include "GanttDiagram.h"
#include "SolutionReport.h"

//#include <boost/graph/graphviz.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <utility>
#include <cstdlib>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;
using namespace boost;

int main(int argc, char *argv[])
{
	if (argc < 2) {
		return 1;
	}

	clock_t begin, end;
	string instance_name(argv[1]);
	instance_name = instance_name.substr(0, instance_name.rfind("."));

	GanttDiagram *gd;

	cerr << "Inizio lettura istanza e generazione insiemi B e G." << endl;
	begin = clock();
	SetModel problemSets(argv[1]);
	end = clock();
	cerr << "  Insiemi B e G generati in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms.\n" << endl;

	cerr << "Generazione grafi di assegnamento (H) e compatibilita' (C) e maximal cliques" << endl;
	begin = clock();
	GraphModel gModel(problemSets);
	end = clock();
	cerr << "  Grafi H e C generati in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms.\n" << endl;

	int *solution = 0;
	int *warmStart = 0;
	string svg_output;

	/*
	ListColoringTreeSearch *lcts = new ListColoringTreeSearch(problemSets);
	lcts->ListColoring(1);
	delete lcts;
	*/

	/*
	 * EURISTICA IntervalPackingFinishFirst
	 */
	cerr << "/*" << endl;
	cerr << " * IntervalPackingFinishFirst" << endl;
	cerr << " */\n" << endl;
	cerr << "Euristica di packing FinishFirst" << endl;
	begin = clock();
	IntervalPackingFinishFirst *finishFirst =
		new IntervalPackingFinishFirst(problemSets, gModel);
	finishFirst->solveX();
	end = clock();
	cerr << "  IntervalPackingFinishFirst in " << (end -
			begin) /
		(double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "finishFirst was successful? " << finishFirst->solved()<< endl;
	
	if (finishFirst->solved()) {
		svg_output = "IntervalPackingFinishFirst.svg";
		finishFirst->solution(solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(),
						  problemSets.B(), solution);
		delete[]solution;
		delete(gd);
		cerr << "    Time: " << finishFirst->elapsedTime() << "ms." << endl;
		cerr << "    Objective function: " << finishFirst->objectiveFunction() << " piattaforme.\n" << endl;
	}
	delete(finishFirst);

	/*
	 * EURISTICA IntervalPackingFinishFirst
	 */
	cerr << "/*" << endl;
	cerr << " * IntervalPackingFirstFit" << endl;
	cerr << " */\n" << endl;
	cerr << "Euristica di packing FirstFit" << endl;
	begin = clock();
	IntervalPackingFirstFit *firstFit =
		new IntervalPackingFirstFit(problemSets, gModel);
	firstFit->solveX();
	end = clock();
	cerr << "  IntervalPackingFirstFit in " << (end -
			begin) /
		(double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "firstFit was successful? " << firstFit->solved()<< endl;
	
	if (firstFit->solved()) {
		svg_output = "IntervalPackingFirstFit.svg";
		firstFit->solution(solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(),
						  problemSets.B(), solution);
		delete[]solution;
		delete(gd);
		cerr << "    Time: " << firstFit->elapsedTime() << "ms." << endl;
		cerr << "    Objective function: " << firstFit->objectiveFunction() << " piattaforme.\n" << endl;
	}
	delete(firstFit);
	
	/*
	 * MathModelColoring
	 */
	cerr << "/*" << endl;	cerr << " * MathModelColoring" << endl;	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico di list-coloring." << endl;
	begin = clock();
	MathModel *colModel = new MathModelColoring(gModel);
	end = clock();
	colModel->writeModelLP_solve("modelColoring.lp");
	colModel->writeModelCPLEX("modelColoring_CPLEX.lp");
	cerr << "  Costruzione di MathModelColoring in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	colModel->verbose(IMPORTANT); // NEUTRAL CRITICAL SEVERE IMPORTANT NORMAL DETAILED FULL
	colModel->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelColoring in " << (end -	begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << colModel->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << colModel->objectiveFunction() << " (" << (sqrt(8*colModel->objectiveFunction()+1)-1) / 2 << ") piattaforme x peso." << endl;
	cerr << "    Iterations: " << colModel->totalIter() << endl;
	cerr << "    Nodes: " << colModel->totalNodes() << "\n" << endl;
	if (colModel->solved()) {
		svg_output = "MathModelColoring.svg";
		colModel->solution(solution);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
//		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		delete[]solution;
		delete(gd);
	}
	delete colModel;

	/*
	 * MathModelBP
	 */
	cerr << "/*" << endl;	cerr << " * MathModelBP" << endl;	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico di list-coloring. Vincoli compattati." << endl;
	begin = clock();
	MathModel *bpModel = new MathModelBP(gModel);
	end = clock();
	bpModel->writeModelLP_solve("modelBP.lp");
	bpModel->writeModelCPLEX("modelBP_CPLEX.lp");
	cerr << "  Costruzione di MathModelBP in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	bpModel->verbose(IMPORTANT); // NEUTRAL CRITICAL SEVERE IMPORTANT NORMAL DETAILED FULL
	bpModel->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelBP in " << (end -	begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << bpModel->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << bpModel->objectiveFunction() << " (" << (sqrt(8*bpModel->objectiveFunction()+1)-1) / 2 << ") piattaforme x peso." << endl;
	cerr << "    Iterations: " << bpModel->totalIter() << endl;
	cerr << "    Nodes: " << bpModel->totalNodes() << "\n" << endl;
	if (bpModel->solved()) {
		svg_output = "MathModelBP.svg";
		bpModel->solution(solution);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
//		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		delete[]solution;
		delete(gd);
	}
	delete bpModel;

	/*
	 * MathModelBPsingle
	 */
	cerr << "/*" << endl;	cerr << " * MathModelBPsingle" << endl;	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico di list-coloring. Tutti i vincoli." << endl;
	begin = clock();
	MathModel *bpModelsingle = new MathModelBPsingle(gModel);
	end = clock();
	bpModelsingle->writeModelLP_solve("modelBPsingle.lp");
	bpModelsingle->writeModelCPLEX("modelBPsingle_CPLEX.lp");
	cerr << "  Costruzione di MathModelBPsingle in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "  Il modello viene risolto in tempo maggiore. Settaggio soluzione iniziale." << endl;
	bpModelsingle->verbose(IMPORTANT);
//	if (warmStart)
//		bpModelsingle->initialSolution(warmStart);
	end = clock();
	cerr << "  Soluzione iniziale per MathModelBPsingle dopo " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	bpModelsingle->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelBPsingle in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << bpModelsingle->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << bpModelsingle->objectiveFunction() << " (" << (sqrt(8*bpModelsingle->objectiveFunction()+1)-1) / 2 << ") piattaforme x peso." << endl;
	cerr << "    Iterations: " << bpModelsingle->totalIter() << endl;
	cerr << "    Nodes: " << bpModelsingle->totalNodes() << "\n" << endl;
	if (bpModelsingle->solved()) {
		svg_output = "MathModelBPsingle.svg";
		bpModelsingle->solution(solution);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
//		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		delete[]solution;
		delete(gd);
	}
	delete bpModelsingle;

	/*
	 *  MathModelMinPConflict
	 */
	cerr << "/*" << endl;	cerr << " * mPconflictModel" << endl;	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico per min conflitto." << endl;
	begin = clock();
	MathModel *mPconflictModel = new MathModelMinPConflict(gModel);
	end = clock();
	mPconflictModel->writeModelLP_solve("modelMinPConflict.lp");
	mPconflictModel->writeModelCPLEX("modelMinPConflict_CPLEX.lp");
	cerr << "  Costruzione di MathModelMinPConflict in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "  Il modello viene risolto in tempo maggiore. Settaggio soluzione iniziale." << endl;
	mPconflictModel->verbose(IMPORTANT);
//	mPconflictModel->initialSolution(warmStart);
	end = clock();
	cerr << "  Soluzione iniziale per MathModelMinPConflict in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	mPconflictModel->setTimeout(900);
	mPconflictModel->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelMinPConflict in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << mPconflictModel->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << mPconflictModel->objectiveFunction() << " (probabilita')" << endl;
	cerr << "    Iterations: " << mPconflictModel->totalIter() << endl;
	cerr << "    Nodes: " << mPconflictModel->totalNodes() << "\n" << endl;
	if (mPconflictModel->solved() || true) {
		svg_output = "MathModelMinPConflict.svg";
		mPconflictModel->solution(solution);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution);

		delete[]solution;
		delete(gd);
	}
	delete mPconflictModel;

	/*
	 * MathModelMaxMinDistance
	 */
	cerr << "/*" << endl;	cerr << " * MathModelMaxMinDistance" << endl;	cerr << " */\n" << endl;
	cerr << "Costruzione del modello per massimizzazione distanze fra soste" << endl;
	begin = clock();
	MathModel *mmdModel = new MathModelMaxMinDistance(gModel);
	end = clock();
	mmdModel->writeModelLP_solve("modelMaxMinDistance.lp");
	mmdModel->writeModelCPLEX("modelMaxMinDistance_CPLEX.lp");
	cerr << "  Costruzione di MathModelMaxMinDistance in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "  Il modello viene risolto in tempo maggiore. Settaggio soluzione iniziale." << endl;
	mmdModel->verbose(IMPORTANT);
//	mmdModel->initialSolution(warmStart);
	end = clock();
	cerr << "  Soluzione iniziale per MathModelMaxMinDistance in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	mmdModel->setTimeout(900);
	mmdModel->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelMaxMinDistance in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << mmdModel->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << mmdModel->objectiveFunction() << endl;
	cerr << "    Iterations: " << mmdModel->totalIter() << endl;
	cerr << "    Nodes: " << mmdModel->totalNodes() << "\n" << endl;
	if (mmdModel->solved() || true) {
		svg_output = "MathModelMaxMinDistance.svg";
		mmdModel->solution(solution);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		delete[]solution;
		delete(gd);
	}
	delete mmdModel;

	delete []warmStart;

	// system("pause");
	
	/*
	begin = clock();
	MathModelMaxTotalDistance *tdModel = new MathModelMaxTotalDistance(gModel);
	tdModel->solveX();
	end = clock();
	cerr << "MathModelMaxTotalDistance in " << (end -
			begin) /
		(double)CLOCKS_PER_SEC << " secondi." << endl;
	svg_output = "MathModelMaxTotalDistance.svg";
	tdModel->solution(solution);
	gd = new GanttDiagram(svg_output.c_str(), problemSets.G(),
					  problemSets.B(), solution);
	delete[]solution;
	delete(gd);
	delete tdModel;
	*/

	return 0;
}
