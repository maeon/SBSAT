/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark,  service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

bool CheckSimpleLimits(double fStartTime) {
	double fEndTime;
	fEndTime = get_runtime();
	if (nCtrlC) {
		return 1;
	}
	
	if (nTimeLimit && (fEndTime - fStartTime) > nTimeLimit) {
		d2_printf2("Bailling out because the Time limit of %lds ", nTimeLimit);
		d2_printf2("is smaller than elapsed time %.0fs\n", (fEndTime - fStartTime));
		return 1;
	}
	      
	if (nNumChoicePointLimit && ite_counters[NUM_CHOICE_POINTS]>nNumChoicePointLimit) {
		d2_printf2("Bailling out because the limit on the number of choicepoints %ld ",
					  nNumChoicePointLimit);
		d2_printf2("is smaller than the number of choicepoints %ld\n", (long)ite_counters[NUM_CHOICE_POINTS]);
		return 1;
	}
	
	return 0;
}

ITE_INLINE void SmurfStates_Push_Hooks() {
//push GE table onto the stack.
//frame allocated here?
//	PrintAllXORSmurfStateEntries();
}

ITE_INLINE void SmurfStates_Pop_Hooks() {
//probably do nothing.

}

ITE_INLINE void Alloc_SmurfStack_Hooks(int destination) {
	//for(int i = destination; i < destination + SMURF_STATES_INCREASE_SIZE && i < SimpleSmurfProblemState->nNumVars; i++) {
	//	SimpleSmurfProblemState->arrSmurfStack[i].XORframe = new_frame();???
	//}
	
	
}

ITE_INLINE int ApplyInference_Hooks(int nBranchVar, bool bBVPolarity) {
	int ret = 1;
	
	
	
	return ret;	
}

ITE_INLINE int ApplyInferenceToSmurf_Hook(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	int ret = 1;
	
	//Addrow could be called here if arrSmurfStates[nSmurfNumber].cType == FN_XOR_GELIM
	//call addrow
	//arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	
	return ret;
}

ITE_INLINE int Backtrack_Hooks() {
	int ret = SOLV_UNKNOWN;
	
	//Periodic Hooks
	if (ite_counters[NUM_BACKTRACKS] % BACKTRACKS_PER_STAT_REPORT == 0) {

		//Display hook
		DisplaySimpleSolverBacktrackInfo(fSimpleSolverPrevEndTime, fSimpleSolverStartTime);

		//cvs trace file hook
		if (fd_csv_trace_file) {
			DisplaySimpleSolverBacktrackInfo_gnuplot(fSimpleSolverPrevEndTime, fSimpleSolverStartTime);
		}
		//Solving limit hook
		if(CheckSimpleLimits(fSimpleSolverStartTime)==1) return SOLV_LIMIT;
	}
	
	return ret;	
}

ITE_INLINE void Init_Solver_PreSmurfs_Hooks() {
	
	
}

ITE_INLINE int Init_Solver_PostSmurfs_Hooks() {
	int ret = SOLV_UNKNOWN;
	
	
	return ret;
}

ITE_INLINE void Final_Solver_Hooks() {
	
	
}