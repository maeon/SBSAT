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

/* 
 * input: none
 *
 */

extern int autarky;

ITE_INLINE void pop_state_information(int n);
ITE_INLINE int pop_mark_state_information();
ITE_INLINE void check_pop_information_init();
ITE_INLINE int check_pop_information_specfn();
ITE_INLINE int check_pop_information_smurfs();
ITE_INLINE int is_autarky();
ITE_INLINE void Mark_arrSmurfStatesStack(int);
ITE_INLINE void Mark_arrNumRHSUnknowns(int);

ITE_INLINE
int
BackTrack_NL()
{
  int nInferredAtom;  /* the choice point */
  int nInferredValue; /* the value of the choice point */

  int loop_counter = 1; /* autarky loop counter */
  int mark_found = 0;   /* found the mark of a forced cp inference */

  if (autarky) {
    mark_found = pop_mark_state_information();
    check_pop_information_init();
  }

 /* autarky loop */
 do 
  {

  // Pop the choice point stack.
  pChoicePointTop--;
  if (pChoicePointTop < arrChoicePointStack)
    {
      //goto_NoSolution;
      return 0;
    }

  /* the choice point */
  nInferredAtom = pChoicePointTop->nBranchVble;

  /* the value of the choice point */
  nInferredValue = arrSolution[nInferredAtom];

#ifdef DISPLAY_TRACE
  if (nNumBacktracks >= TRACE_START)
    {
      cout << "Preparing to backtrack from choice assignment of X"
           <<  nInferredAtom << " equal to "
           << (nInferredValue == BOOL_TRUE ? "true" : "false");
    }
#endif
 
  // Pop the num unresolved functions 
  nNumUnresolvedFunctions = pChoicePointTop->nNumUnresolved;

  // Pop heuristic scores.
  if (nHeuristic == JOHNSON_HEURISTIC || nHeuristic == STATE_HEURISTIC)
	J_PopHeuristicScores();

  // Pop the backtrack stack until we pop the branch atom.
  while (1)
    {
      pBacktrackTop--;
      nBacktrackStackIndex--;
      assert(nBacktrackStackIndex >= 0);
      
      /*m invalidating arrBacktrackStackIndex but keep the prev value */
      int nBacktrackAtom = pBacktrackTop->nAtom;
      arrBacktrackStackIndex[nBacktrackAtom] = gnMaxVbleIndex + 1;
      
      if (nBacktrackAtom == nInferredAtom) 
	{
	  break;
	} 
      
#ifdef DISPLAY_TRACE
      if (nNumBacktracks >= TRACE_START)
	{
	  cout << "Backtracking from forced assignment of X"
	       << nBacktrackAtom << " equal to "
	       << (arrSolution[nBacktrackAtom] == BOOL_TRUE ? "true" : "false")
	       << "." << endl;
	}
#endif
      
      arrSolution[nBacktrackAtom] = BOOL_UNKNOWN;
    };  //  while (1)

    arrSolution[nInferredAtom] = BOOL_UNKNOWN;

    if (autarky && mark_found) {
       mark_found = 0;
       while (is_autarky()) loop_counter++;

       if (loop_counter>1) 
         { d2_printf2("--------------A (%d)--------------\n", loop_counter-1); };
    }; 

    pop_state_information(1);

    loop_counter--;
 } while (loop_counter > 0);


  if (autarky) {
    nCurSmurfStatesVersion++;
    Mark_arrSmurfStatesStack(LEVEL_MARK); /* mark the reversing polarity */

    nCurSpecialFnVersion++;
    Mark_arrNumRHSUnknowns(LEVEL_MARK); /* mark the reversing polarity */
  }

  // Flush the inference queue.
  pInferenceQueueNextElt = pInferenceQueueNextEmpty = arrInferenceQueue;
  pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;

  // Reverse the polarity of the branch atom.
  nInferredValue = (nInferredValue == BOOL_TRUE ? BOOL_FALSE : BOOL_TRUE);

  InferLiteral(nInferredAtom, nInferredValue, true, NULL, NULL, 1);

#ifdef DISPLAY_TRACE
  if (nNumBacktracks >= TRACE_START)
    {
      cout << "Reversed polarity of X" << nInferredAtom
      << " to " << nInferredValue << endl;
      //DisplayAllBrancherLemmas();
      cout << "Would DisplayAllBrancherLemmas here." << endl;
    }
#endif  
  
  // Get the consequences of the branch atoms new value.
  return 1;
}

