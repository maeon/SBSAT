/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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
#include "ite.h"
#include "solver.h"

extern int nNumChoicePts;
extern int nNumVariables;
extern int gnMaxVbleIndex;
extern SmurfState **arrCurrentStates;
extern int nNumRegSmurfs;
extern int nNumBytesInStateArray;
extern int nNumBytesInSpecialFuncStackEntry;
extern ChoicePointStruct *pChoicePointTop;
extern int nNumUnresolvedFunctions;
extern int nNumBacktracks;
extern int *arrBacktrackStackIndex;
extern int nBacktrackStackIndex;
extern int *pInferenceQueueNextEmpty;
extern int zecc_limit;
extern int *zecc_arr;


ITE_INLINE void push_smurf_states_onto_stack();
ITE_INLINE void push_special_fn_onto_stack();


ITE_INLINE void
SelectNewBranchPoint()
{
   // We need to select a new branch point.
   D_9(
         if (nNumBacktracks >= TRACE_START)
         {/*
           DisplayStatus(nNumRegSmurfs, arrCurrentStates,
           nNumUnresolvedFunctions,
           nNumChoicePts, nNumBacktracks);
           DisplayPartialSolution(nNumVariables, arrSolution);
           */
         }
      )
      d9_printf2("Calling heuristic to choose choice point #%d\n", nNumChoicePts);

   if (nHeuristic == JOHNSON_HEURISTIC)
      J_UpdateHeuristic();
   else
      UpdateHeuristic();

   // Call heuristic.
   int nInferredAtom;
   int nInferredValue;
   proc_call_heuristic(&nInferredAtom, &nInferredValue);

   /*	
   //override heuristic for first so many variables...
   //Special Zecchina code...Sean play!
   for(int zecc = 0; zecc < zecc_limit; zecc++) {
   if(zecc_arr[zecc]>0) {
   if(arrSolution[zecc_arr[zecc]] == BOOL_UNKNOWN) {
   nInferredAtom = zecc_arr[zecc];
   nInferredValue = BOOL_TRUE;
   break;
   }
   } else {
   if(arrSolution[-zecc_arr[zecc]] == BOOL_UNKNOWN) {
   nInferredAtom = -zecc_arr[zecc];
   nInferredValue = BOOL_FALSE;
   break;
   }
   }
   }
    */	

   assert(nInferredValue == BOOL_TRUE || nInferredValue == BOOL_FALSE);
   assert(arrSolution[nInferredAtom] == BOOL_UNKNOWN);

   if (nInferredAtom > nIndepVars && nInferredAtom <= nDepVars)
      ite_counters[HEU_DEP_VAR]++;

   nNumChoicePts++;

   push_smurf_states_onto_stack();
   push_special_fn_onto_stack();

   // Push branch atom onto stack.
   pChoicePointTop->nBranchVble = nInferredAtom;

   // Push num unresolved functions
   pChoicePointTop->nNumUnresolved = nNumUnresolvedFunctions;

   // Push heuristic scores.
   if (nHeuristic == JOHNSON_HEURISTIC)
      J_PushHeuristicScores();

   pChoicePointTop++;
   pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;

   InferLiteral(nInferredAtom, nInferredValue, false, NULL, NULL, 0);
   return;
}

