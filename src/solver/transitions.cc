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

extern int *arrIte2SolverVarMap;

ITE_INLINE Transition *
AddStateTransition(SmurfState *pSmurfState,
                   int i,
                   int nVble,
                   int nValueOfVble,
                   BDDNodeStruct *pFuncEvaled,
                   SmurfState *pSmurfStateOfEvaled
                   )
{
  Transition *pTransition = FindOrAddTransition(pSmurfState, i, nVble, nValueOfVble);

  assert(pTransition->pNextState == NULL);
  assert(pTransition->pState == NULL);

  pTransition->pNextState = pSmurfStateOfEvaled;
  pTransition->pState = pSmurfState;

  int size = pFuncEvaled->addons->pImplied->GetPtrPositiveInferences()->Size() +
             pFuncEvaled->addons->pImplied->GetPtrNegativeInferences()->Size();
  int *ptr = NULL;
  if (size) {
     ptr = (int*)ite_calloc(size, sizeof(int), 9, "inferences");

     // Take the positive inferences from *pFuncEvaled and store them
     // in the transition.
     pFuncEvaled->addons->pImplied->GetPtrPositiveInferences()
        ->StoreAsArrayBasedSet(pTransition->positiveInferences, ptr);
     ptr += pTransition->positiveInferences.nNumElts;

     for(int i=0;i<pTransition->positiveInferences.nNumElts;i++) {
        pTransition->positiveInferences.arrElts[i] = 
           arrIte2SolverVarMap[pTransition->positiveInferences.arrElts[i]];
     }

     // Likewise for the negative inferences.
     pFuncEvaled->addons->pImplied->GetPtrNegativeInferences()
        ->StoreAsArrayBasedSet(pTransition->negativeInferences, ptr);

     for(int i=0;i<pTransition->negativeInferences.nNumElts;i++) {
        pTransition->negativeInferences.arrElts[i] = 
           arrIte2SolverVarMap[pTransition->negativeInferences.arrElts[i]];
     }
  }
  return pTransition;
}


ITE_INLINE Transition *
FindTransitionDebug (SmurfState * pState, int i, int nVble, int nVbleValue)
{
  assert (pState != pTrueSmurfState);
  assert (pState->vbles.nNumElts > i);
  assert (pState->vbles.arrElts[i] == nVble);
  assert (nVbleValue == BOOL_TRUE || nVbleValue == BOOL_FALSE);
  assert (pState->arrTransitions);
  return pState->arrTransitions + 2 * i + nVbleValue;
}

