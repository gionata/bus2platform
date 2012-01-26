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
#include "ConstraintProgrammingListGraphColoring.h"
#include "MathModelBP.h"
#include "MathModelBPsingle.h"
#include "MathModelColoring.h"
#include "MathModelMaxMinDistance.h"
#include "MathModelMinPConflict.h"
#include "IntervalPackingFinishFirst.h"
#include "IntervalPackingFirstFit.h"
#include "IterativeTimeHorizonMath.h"
#include "IterativeTimeHorizonMathMD.h"
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

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000
#endif

#include <omp.h>

using namespace std;
using namespace boost;

bool intervalPackingFirstFirst(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool intervalPackingFinishFirst(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool mathModelColoring(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool mathModelBP(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool mathModelBPsingle(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool mathModelMinPConflict(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool mathModelMaxMinDistance(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool iterativeTimeHorizonMath(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool iterativeTimeHorizonMathMD(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool real_solution(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);
bool cp_listcoloring(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name);

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
	SetModel problemSets(argv[1], 0);
	end = clock();
	cerr << "  Insiemi B e G generati in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms.\n" << endl;

	cerr << "Generazione grafi di assegnamento (H) e compatibilita' (C) e maximal cliques" << endl;
	begin = clock();
	GraphModel gModel(problemSets);
	end = clock();
	cerr << "  Grafi H e C generati in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms.\n" << endl;

	int *warmStart = 0;

	string reference_name("istanza_reale_feasible");
	size_t ref_len = reference_name.length();
	size_t inst_len = instance_name.length();

	if (
		(ref_len==inst_len && instance_name.compare(reference_name) == 0) ||
		(
		ref_len<inst_len &&
			(
		instance_name.substr(inst_len-ref_len, ref_len).compare(reference_name) == 0 ||
	        instance_name.substr(inst_len-ref_len-4, ref_len).compare(reference_name) == 0
			)
		)
	   )
		real_solution(problemSets, gModel, warmStart, instance_name);

	#pragma omp parallel
	{
		#pragma omp single nowait
		{
			#pragma omp task
			warmStart = 0;
			mathModelColoring(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			warmStart = 0;
			mathModelBP(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			warmStart = 0;
			mathModelBPsingle(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			warmStart = 0;
			iterativeTimeHorizonMath(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			warmStart = 0;
			iterativeTimeHorizonMathMD
			(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			intervalPackingFirstFirst(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			intervalPackingFinishFirst(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			warmStart = 0;
			cp_listcoloring(problemSets, gModel, warmStart, instance_name);


			#pragma omp task
			warmStart = 0;
			mathModelMinPConflict(problemSets, gModel, warmStart, instance_name);

			#pragma omp task
			warmStart = 0;
			mathModelMaxMinDistance(problemSets, gModel, warmStart, instance_name);
		}
	}
	#pragma omp taskwait

	delete []warmStart;

	/*
	ListColoringTreeSearch *lcts = new ListColoringTreeSearch(problemSets);
	lcts->ListColoring(1);
	delete lcts;
	*/

	// system("pause");
	// system("read");

	return 0;
}

bool intervalPackingFirstFirst(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 * EURISTICA IntervalPackingFirstFirst
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
	cerr << "firstFit was successful? " << (bool) firstFit->solved() << endl;

	if (ret = (firstFit->solved())) {
		svg_output = "IntervalPackingFirstFit.svg";
		pdf_output = "IntervalPackingFirstFit.pdf";
		firstFit->solution(solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);

		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(),
		                      problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, firstFit->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(),
		                      problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, firstFit->elapsedTime(), pdf);
		delete(gd);
		delete[]solution;
		cerr << "    Time: " << firstFit->elapsedTime() << "ms." << endl;
		cerr << "    Objective function: " << firstFit->objectiveFunction() << " piattaforme.\n" << endl;
	}
	delete(firstFit);

	return ret;
}

bool intervalPackingFinishFirst(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

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
	cerr << "finishFirst was successful? " << (bool) finishFirst->solved() << endl;

	if (ret = (finishFirst->solved())) {
		svg_output = "IntervalPackingFinishFirst.svg";
		pdf_output = "IntervalPackingFinishFirst.pdf";
		finishFirst->solution(solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(),
		                      problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, finishFirst->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(),
		                      problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, finishFirst->elapsedTime(), pdf);
		delete(gd);
		delete[]solution;
		cerr << "    Time: " << finishFirst->elapsedTime() << "ms." << endl;
		cerr << "    Objective function: " << finishFirst->objectiveFunction() << " piattaforme.\n" << endl;
	}
	delete(finishFirst);

	return ret;
}

bool mathModelColoring(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 * MathModelColoring
	 */
	cerr << "/*" << endl;
	cerr << " * MathModelColoring" << endl;
	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico di list-coloring." << endl;
	begin = clock();
	MathModel *colModel = new MathModelColoring(gModel);
	end = clock();
	colModel->writeModelLP_solve("modelColoring.lp");
	colModel->writeModelCPLEX("modelColoring_CPLEX.lp");
	cerr << "  Costruzione di MathModelColoring in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	colModel->verbose(IMPORTANT); // NEUTRAL CRITICAL SEVERE IMPORTANT NORMAL DETAILED FULL
	colModel->setTimeout(5);
	colModel->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelColoring in " << (end -	begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << colModel->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << colModel->objectiveFunction() << " (" << (sqrt(8*colModel->objectiveFunction()+1)-1) / 2 << ") piattaforme x peso." << endl;
	cerr << "    Iterations: " << colModel->totalIter() << endl;
	cerr << "    Nodes: " << colModel->totalNodes() << "\n" << endl;
	if (ret = (colModel->solved())) {
		svg_output = "MathModelColoring.svg";
		pdf_output = "MathModelColoring.pdf";
		colModel->solution(solution);
		colModel->solutionFeasibility(solution, "MathModelColoring");
		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);

		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, colModel->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, colModel->elapsedTime(), pdf);
		delete(gd);

		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));

		delete[]solution;
	}
	delete colModel;

	return ret;
}

bool mathModelBP(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 * MathModelBP
	 */
	cerr << "/*" << endl;
	cerr << " * MathModelBP" << endl;
	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico di list-coloring. Vincoli compattati." << endl;
	begin = clock();
	MathModel *bpModel = new MathModelBP(gModel);
	end = clock();
	 bpModel->setTimeout(5);
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
	if (ret = (bpModel->solved())) {
		svg_output = "MathModelBP.svg";
		pdf_output = "MathModelBP.pdf";
		bpModel->solution(solution);
		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);

		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, bpModel->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, bpModel->elapsedTime(), pdf);
		delete(gd);

		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		bpModel->solutionFeasibility(solution, "MathModelBP");

		delete[]solution;
	}
	delete bpModel;

	return ret;
}

bool mathModelBPsingle(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;
	warmStart = 0;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 * MathModelBPsingle
	 */
	cerr << "/*" << endl;
	cerr << " * MathModelBPsingle" << endl;
	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico di list-coloring. Tutti i vincoli." << endl;
	begin = clock();
	MathModel *bpModelsingle = new MathModelBPsingle(gModel);
	end = clock();
	 bpModelsingle->setTimeout(5);
	bpModelsingle->writeModelLP_solve("modelBPsingle.lp");
	bpModelsingle->writeModelCPLEX("modelBPsingle_CPLEX.lp");
	cerr << "  Costruzione di MathModelBPsingle in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "  Il modello viene risolto in tempo maggiore. Settaggio soluzione iniziale." << endl;
	bpModelsingle->verbose(IMPORTANT);
	if (warmStart)
		bpModelsingle->initialSolution(warmStart);
	end = clock();
	cerr << "  Soluzione iniziale per MathModelBPsingle dopo " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	bpModelsingle->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelBPsingle in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << bpModelsingle->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << bpModelsingle->objectiveFunction() << " (" << (sqrt(8*bpModelsingle->objectiveFunction()+1)-1) / 2 << ") piattaforme x peso." << endl;
	cerr << "    Iterations: " << bpModelsingle->totalIter() << endl;
	cerr << "    Nodes: " << bpModelsingle->totalNodes() << "\n" << endl;
	if (ret = (bpModelsingle->solved())) {
		svg_output = "MathModelBPsingle.svg";
		pdf_output = "MathModelBPsingle.pdf";
		bpModelsingle->solution(solution);
		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, bpModelsingle->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, bpModelsingle->elapsedTime(), pdf);
		delete(gd);

		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		bpModelsingle->solutionFeasibility(solution, "MathModelBPsingle");
		delete[]solution;
	}
	delete bpModelsingle;

	return ret;
}

bool mathModelMinPConflict(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 *  MathModelMinPConflict
	 */
	cerr << "/*" << endl;
	cerr << " * mPconflictModel" << endl;
	cerr << " */\n" << endl;
	cerr << "Costruzione del modello matematico per min conflitto." << endl;
	begin = clock();
	MathModel *mPconflictModel = new MathModelMinPConflict(gModel);
	end = clock();
	mPconflictModel->writeModelLP_solve("modelMinPConflict.lp");
	mPconflictModel->writeModelCPLEX("modelMinPConflict_CPLEX.lp");
	cerr << "  Costruzione di MathModelMinPConflict in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "  Il modello viene risolto in tempo maggiore. Settaggio soluzione iniziale." << endl;
	mPconflictModel->verbose(NORMAL);
	if (warmStart)
		mPconflictModel->initialSolution(warmStart);
	end = clock();
	cerr << "  Soluzione iniziale per MathModelMinPConflict in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	mPconflictModel->setTimeout(10); // 1800
	mPconflictModel->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelMinPConflict in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << mPconflictModel->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << mPconflictModel->objectiveFunction() << " (probabilita')" << endl;
	cerr << "    Iterations: " << mPconflictModel->totalIter() << endl;
	cerr << "    Nodes: " << mPconflictModel->totalNodes() << "\n" << endl;
	if (ret = (mPconflictModel->solved())) {
		svg_output = "MathModelMinPConflict.svg";
		pdf_output = "MathModelMinPConflict.pdf";
		mPconflictModel->solution(solution);

		mPconflictModel->solutionFeasibility(solution, "MathModelMinPConflict");

		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mPconflictModel->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mPconflictModel->elapsedTime(), pdf);
		delete(gd);
		delete[]solution;
	}
	delete mPconflictModel;

	return ret;
}

bool mathModelMaxMinDistance(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 * MathModelMaxMinDistance
	 */
	cerr << "/*" << endl;
	cerr << " * MathModelMaxMinDistance" << endl;
	cerr << " */\n" << endl;
	cerr << "Costruzione del modello per massimizzazione distanze fra soste" << endl;
	begin = clock();
	MathModel *mmdModel = new MathModelMaxMinDistance(gModel);
	end = clock();
	mmdModel->writeModelLP_solve("modelMaxMinDistance.lp");
	mmdModel->writeModelCPLEX("modelMaxMinDistance_CPLEX.lp");
	cerr << "  Costruzione di MathModelMaxMinDistance in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "  Il modello viene risolto in tempo maggiore. Settaggio soluzione iniziale." << endl;
	mmdModel->verbose(NORMAL);//IMPORTANT
	if (warmStart)
		mmdModel->initialSolution(warmStart);
	end = clock();
	cerr << "  Soluzione iniziale per MathModelMaxMinDistance in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	mmdModel->setTimeout(10); //1800
	mmdModel->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelMaxMinDistance in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	cerr << "    Time (lp->solve()): " << mmdModel->elapsedTime() << "ms." << endl;
	cerr << "    Objective function: " << mmdModel->objectiveFunction() << endl;
	cerr << "    Iterations: " << mmdModel->totalIter() << endl;
	cerr << "    Nodes: " << mmdModel->totalNodes() << "\n" << endl;
	if (ret = (mmdModel->solved())) {
		svg_output = "MathModelMaxMinDistance.svg";
		pdf_output = "MathModelMaxMinDistance.pdf";
		mmdModel->solution(solution);
		if (!warmStart) {
			warmStart = new int[problemSets.B().size()];
		}
		memcpy(warmStart, solution, problemSets.B().size() * sizeof(int));
		mmdModel->solutionFeasibility(solution, "MathModelMaxMinDistance");

		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mmdModel->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mmdModel->elapsedTime(), pdf);
		delete(gd);
		delete[]solution;

	}
	delete mmdModel;

	return ret;
}

bool iterativeTimeHorizonMath(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 *  IterativeTimeHorizonMath
	 */
	cerr << "/*" << endl;
	cerr << " * mIterativeTimeHorizonMath" << endl;
	cerr << " */\n" << endl;
	cerr << "Risoluzione iterativa del modello matematico per min conflitto." << endl;
	begin = clock();
	IterativeTimeHorizonMath *mIterativeTimeHorizonMath = new IterativeTimeHorizonMath(gModel);
	mIterativeTimeHorizonMath->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelMinPConflict in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	if (ret = (mIterativeTimeHorizonMath->solved())) {
		svg_output = "IterativeTimeHorizonMath.svg";
		pdf_output = "IterativeTimeHorizonMath.pdf";
		mIterativeTimeHorizonMath->solution(solution);
		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mIterativeTimeHorizonMath->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mIterativeTimeHorizonMath->elapsedTime(), pdf);
		delete(gd);

		gModel.solutionFeasibility(solution, "IterativeMPC");
		delete[]solution;

	}
	delete mIterativeTimeHorizonMath;

	return ret;
}

bool iterativeTimeHorizonMathMD(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 *  IterativeTimeHorizonMathMD
	 */
	cerr << "/*" << endl;
	cerr << " * mIterativeTimeHorizonMathMD" << endl;
	cerr << " */\n" << endl;
	cerr << "Risoluzione iterativa del modello matematico per max min distance." << endl;
	begin = clock();
	IterativeTimeHorizonMathMD *mIterativeTimeHorizonMath = new IterativeTimeHorizonMathMD(gModel);
	mIterativeTimeHorizonMath->solveX();
	end = clock();
	cerr << "  Soluzione del MathModelMinPConflictMD in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	if (ret = (mIterativeTimeHorizonMath->solved())) {
		svg_output = "IterativeTimeHorizonMathMD.svg";
		pdf_output = "IterativeTimeHorizonMathMD.pdf";
		mIterativeTimeHorizonMath->solution(solution);
		unsigned int used_platform;
		unsigned int min_interval_distance;
		double cprob_lin;
		double cprob_exp;
		double mean_interval;
		//std::cout << "Calcolo gli indicatori" << std::endl;
		problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
		gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mIterativeTimeHorizonMath->elapsedTime(), svg);
		delete(gd);
		gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, mIterativeTimeHorizonMath->elapsedTime(), pdf);
		delete(gd);
		gModel.solutionFeasibility(solution, "IterativeMPC_MD");
		delete[]solution;

	}
	delete mIterativeTimeHorizonMath;

	return ret;
}

bool real_solution(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	GanttDiagram *gd;

	int id_Corsa[] = {33, 34, 35, 39, 40, 41, 42, 43, 44, 45, 46, 48, 49, 50, 51, 52, 53, 54, 55, 57, 58, 59, 60, 61, 68, 69, 70, 74, 76, 82, 85, 87, 91, 93, 94, 96, 100, 102, 103, 104, 105, 106, 107, 108, 148, 149, 150, 151, 152, 153, 154, 155, 156, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 170, 171, 173, 174, 175, 177, 178, 179, 180, 181, 182, 200, 220, 233, 235, 236, 237, 238, 239, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 319, 344, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 369, 370, 371, 372, 373, 374, 376, 377, 378, 379, 392, 393, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 421, 422, 423, 424, 425, 426, 427, 428, 429, 431, 432, 433, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 550, 552, 553, 559, 728, 1121, 1334, 1338, 1404, 1405, 1406, 1408, 1410, 1438, 1521, 1524, 1530, 1611, 1816};

	int id_Gate[] = {12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 10, 10, 9, 9, 9, 9, 9, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 7, 6, 11, 11, 10, 11, 11, 11, 11, 11, 11, 11, 11, 7, 12, 12, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 8, 7, 7, 7, 16, 16, 7, 16, 16, 16, 16, 16, 16, 16, 16, 16, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 17, 22, 18, 17, 18, 17, 17, 24, 17, 23, 17, 18, 18, 18, 18, 22, 18, 18, 18, 18, 17, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 19, 19, 19, 19, 19, 19, 19, 19, 8, 10, 7, 6, 20, 18, 13, 9, 8, 8, 8, 8, 16, 16, 9, 19, 6, 21, 15};

	//for (int i = 0; i < problemSets.G().size(); i++)
	//	cout << problemSets.G()[i]->gateNumber() << endl;

	solution = new int[problemSets.B().size()];
	for (int i = 0; i < problemSets.B().size(); i++) {
		for (int j = 0; j < problemSets.B().size(); j++) {
			if ( id_Corsa[i] == problemSets.B()[j]->dwellNumber() ) {
				solution[j] = id_Gate[i] - 1 - 4;
				break;
			}
		}
	}

	//for (int i = 0; i < problemSets.B().size(); i++) {
	//	cout << problemSets.B()[i]->dwellNumber() << " " << problemSets.G()[solution[i]]->gateNumber() << endl;
	//}


	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	cerr << "/*" << endl;
	cerr << " * Real provided solution" << endl;
	cerr << " */\n" << endl;
	cerr << "..." << endl;

	unsigned int used_platform;
	unsigned int min_interval_distance;
	double cprob_lin;
	double cprob_exp;
	double mean_interval;
	problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
	svg_output = "reale.svg";
	pdf_output = "reale.pdf";
	gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, 0, svg);
	delete(gd);
	gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, 0, pdf);
	delete(gd);
	gModel.solutionFeasibility(solution, "reale");
	delete[]solution;

	return ret;
}

bool cp_listcoloring(SetModel &problemSets, GraphModel &gModel, int *&warmStart, string instance_name)
{
	int *solution = 0;
	string svg_output;
	string pdf_output;
	bool ret = false;
	clock_t begin, end, start, stop;
	GanttDiagram *gd;

	// per sicurezza:
	for (Buses::iterator dwellItr = problemSets.B().begin(); dwellItr != problemSets.B().end(); dwellItr++)
		(*dwellItr)->assigned(false);

	/*
	 *  ConstraintProgrammingListGraphColoring
	 */
	cerr << "/*" << endl;
	cerr << " * ConstraintProgrammingListGraphColoring" << endl;
	cerr << " */\n" << endl;
	cerr << "Risoluzione iterativa del modello di programmazione per vincoli." << endl;
	begin = clock();

	//adattamento dei dati
	//creo un vettore contenente gli archi nel formato che ci serve e poi riverso in un array
	/*vector<int> spigoli;
	GraphC g = gModel.graphC();
	graph_traits < GraphC >::edge_iterator eiC, edge_endC;
	for (tie(eiC, edge_endC) = edges( g ); eiC != edge_endC; ++eiC) {
			spigoli.push_back(source(*eiC, g));
		  	spigoli.push_back(target(*eiC, g));
	}
	spigoli.push_back(-1); spigoli.push_back(-1); //servono a gecode

	int * array_spigoli = (int *)malloc(sizeof(int) * spigoli.size());

	for(int i=0; i<spigoli.size();i++){
		array_spigoli[i]=spigoli[i];
	}*/

	//creo un vettore contenente le cliques nel formato che ci serve e poi lo riverso in un array
	//3 x1 x2 x3
	//2 y1 y2
	//4 z1 z2 z3 z4
	//-1 (fine)
	std::vector < std::vector < int >*> allmaxcliques = problemSets.findAllMaximalCliques();
	vector<int> cliques;
	for(std::vector < std::vector < int >*>::const_iterator cliqueptritr = allmaxcliques.begin(); cliqueptritr != allmaxcliques.end(); cliqueptritr++) {
		cliques.push_back((*cliqueptritr)->size());
		for(std::vector < int >::const_iterator interfering_vertex = (*cliqueptritr)->begin(); interfering_vertex != (*cliqueptritr)->end(); interfering_vertex++)
			cliques.push_back( *interfering_vertex);
	}
	cliques.push_back(-1);//serve a gecode

	int* array_cliques = (int *)malloc(sizeof(int) * cliques.size());

	for(int i=0; i<cliques.size(); i++) {
		array_cliques[i]=cliques[i];
	}
	//fine adattamento dei dati
	int **inadmissibleColors = new int *[problemSets.B().size()];
	for (int i = 0; i < problemSets.B().size(); i++) {
		inadmissibleColors[i] = new int [problemSets.G().size()+1];
		int k = 0;
		for (int j = 0; j < problemSets.G().size(); j++) {
//			cout << problemSets.B()[i]->dwellNumber() << " " << problemSets.G()[j]->gateNumber() << " " << problemSets.B()[i]->compatible(problemSets.G()[j]) << endl;
			if(!problemSets.B()[i]->compatible(problemSets.G()[j]))
				inadmissibleColors[i][k++] = j;
		}
		inadmissibleColors[i][k] = -1;
	}

	/*for (int i = 0; i < problemSets.B().size(); i++) {
		cout << "Inamm " << i << ": ";
		for (int j = 0; j < problemSets.B()[i]->gates().size() && inadmissibleColors[i][j] != -1; j++) {
			cout << inadmissibleColors[i][j] << " ";
		}
		cout << endl;
	}*/

	GraphColorSpec g1(problemSets.B().size(), array_cliques, inadmissibleColors, problemSets.lowerBoundNumberGates());
	SizeOptions opt("GraphColor");
	opt.icl(ICL_DOM);
	opt.iterations(20);
	opt.solutions(1);
	//opt.model(GraphColor::MODEL_NONE);
	//opt.model(GraphColor::MODEL_NONE, "none", "no lower bound");
	opt.model(GraphColor::MODEL_CLIQUE, "clique", "use maximal clique size as lower bound");
	//opt.branching(GraphColor::BRANCH_DEGREE);
	//opt.branching(GraphColor::BRANCH_DEGREE, "degree");
	//opt.branching(GraphColor::BRANCH_SIZE, "size");
	opt.branching(GraphColor::BRANCH_SIZE_DEGREE, "sizedegree");
	//opt.branching(GraphColor::BRANCH_SIZE_AFC, "sizeafc");
	//opt.parse(argc,argv);
	//Script::run<GraphColor,BAB,SizeOptions>(opt);
	GraphColor *model = new GraphColor(opt, g1);
	BAB<GraphColor> alg(model);
	start = clock();
	GraphColor *sol = alg.next();
	stop = clock();
	sol->print(std::cout);

	end = clock();
	cerr << "  Soluzione del ConstraintProgrammingListGraphColoring in " << (end - begin) / (double)(CLOCKS_PER_SEC / 1000) << "ms." << endl;
	//if (ret = (mIterativeTimeHorizonMath->solved())) {

	svg_output = "ConstraintProgrammingListGraphColoring.svg";
	pdf_output = "ConstraintProgrammingListGraphColoring.pdf";
	solution = new int[problemSets.B().size()];
	sol->solution(solution);
	gModel.solutionFeasibility(solution, "ConstraintProgrammingListGraphColoring");
	unsigned int used_platform = 0;
	unsigned int min_interval_distance = 0;
	double cprob_lin;
	double cprob_exp;
	double mean_interval = 0;
	//std::cout << "Calcolo gli indicatori" << std::endl;
	problemSets.performances_airo2011(solution, used_platform, min_interval_distance, cprob_lin, cprob_exp, mean_interval);
	gd = new GanttDiagram(svg_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, (stop - start) / (double)CLOCKS_PER_SEC, svg);
	delete(gd);

	gd = new GanttDiagram(pdf_output.c_str(), problemSets.G(), problemSets.B(), solution, used_platform, min_interval_distance, mean_interval, (stop - start) / (double)CLOCKS_PER_SEC, pdf);
	delete(gd);
	delete[]solution;
	//}
	delete model;

	return ret;
}
