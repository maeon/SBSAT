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
 * input: pConflictLemma (used in ConstructTempLemma )
 * output:
 */

#define LEMMA_CONSTANT 10.0

LemmaBlock * pConflictLemma=NULL;
LemmaInfoStruct * pConflictLemmaInfo=NULL;
bool *arrLemmaFlag=NULL;
int  *arrTempLemma=NULL;

ITE_INLINE
int
BackTrack()
{
   int nOldBacktrackStackIndex=0;
   LemmaInfoStruct *pUnitLemmaListTail = NULL;
   int nUnsetLemmaFlagIndex = 0; /* literals that we need to unset after bt */
   LemmaBlock *pNewLemma=NULL; /* last lemma added */
   int nTempLemmaIndex = 0; /* the length of the new/temporary lemma */
   int nNumForcedInfsBelowCurrentCP = 0;
   int nInferredAtom; /* the choice point */
   int nInferredValue; /* the value of the choice point */

   assert(pUnitLemmaList->pNextLemma[0] == NULL);

   // Copy the conflict lemma into arrTempLemma.
   nTempLemmaIndex = ConstructTempLemma();

   /* backjumping loop */
   do {

      // Pop the choice point stack.
      pChoicePointTop--;

      if (pChoicePointTop < arrChoicePointStack)
      {
         //goto_NoSolution;
         return 0;
      }
      nInferredAtom = pChoicePointTop->nBranchVble;
      nInferredValue = arrSolution[nInferredAtom];

      /*
       * reconstruct nNumForcedInfsBelowCurrentCP
       */
      /* cp1 inf1_1 inf1_2 inf1_3 cp2 inf2_1 inf2_2 inf2_3 cp3 ... cp4 ... cp5 */
      nNumForcedInfsBelowCurrentCP = 0;
      for (int i=0; i<nTempLemmaIndex;i++) {
         int nConflictVble = abs(arrTempLemma[i]);
         /* for all lower than current cp */
         /* bigger than cpX .... */
         if(arrBacktrackStackIndex[nConflictVble] >
               arrBacktrackStackIndex[nInferredAtom] && (
                  /* and just those that I'm going to hit for sure on this level */
                  /* lower than last cp  =>  ... cp(X-1) */
                  nOldBacktrackStackIndex == 0 ||
                  arrBacktrackStackIndex[nConflictVble] < nOldBacktrackStackIndex))
            nNumForcedInfsBelowCurrentCP++;
      }
      nOldBacktrackStackIndex = arrBacktrackStackIndex[nInferredAtom];

      // Pop the num unresolved functions 
      nNumUnresolvedFunctions = pChoicePointTop->nNumUnresolved;

      pop_state_information(1);

      // Pop heuristic scores.
      if (nHeuristic == JOHNSON_HEURISTIC || nHeuristic == STATE_HEURISTIC)
         J_PopHeuristicScores();

      // Pop the backtrack stack until we pop the branch atom.
      while (1)
      {
         pBacktrackTop--;
         nBacktrackStackIndex--;
         assert(nBacktrackStackIndex >= 0);

         if (pBacktrackTop->pLemmaInfo && 
               pBacktrackTop->pLemmaInfo->nBacktrackStackReferences > 0)
         {
            // The lemma for this backtrack stack entry was a cached lemma.
            // Decrement the reference count for it.
            // (When the count reaches zero, the lemma can be recycled.)
            (pBacktrackTop->pLemmaInfo->nBacktrackStackReferences)--;

            // Move lemma to front of lemma priority queue.
            MoveToFrontOfLPQ(pBacktrackTop->pLemmaInfo);

            pBacktrackTop->pLemmaInfo = NULL;
         }

         /*m invalidating arrBacktrackStackIndex but keep the prev value */
         int nBacktrackAtom = pBacktrackTop->nAtom;
         int nBacktrackAtomStackIndex = arrBacktrackStackIndex[nBacktrackAtom];
         arrBacktrackStackIndex[nBacktrackAtom] = gnMaxVbleIndex + 1;


         //Handle old choicepoints
         /*m is atom relevant */
         if(arrLemmaFlag[nBacktrackAtom]) {
            nNumForcedInfsBelowCurrentCP--;
         }

         if (pBacktrackTop->bWasChoicePoint == true || 
               nNumForcedInfsBelowCurrentCP == 0 ||
               nBacktrackAtom == nInferredAtom) 
         {
            //nBacktrackAtom is a UIP-unique implication point
            if (pBacktrackTop->bWasChoicePoint == false) {
               pBacktrackTop->pUnitLemmaList = NULL;
            }

            pBacktrackTop->bWasChoicePoint = false;
            if (arrLemmaFlag[nBacktrackAtom])
            {
               // Resolve out lemma literals which are not needed.
               // Do this by checking the level at which each literal
               // was inferred.
               int nTempLemmaLiteral;
               int nTempLemmaVble;

               int nLemmaScore=0;  //Used for scoring lemmas

               int nCopytoIndex = 0;
               for (int i = 0; i < nTempLemmaIndex; i++)
               {
                  nTempLemmaLiteral = arrTempLemma[i];
                  nTempLemmaVble = abs(nTempLemmaLiteral);
                  if ((arrBacktrackStackIndex[nTempLemmaVble] 
                           <= nBacktrackAtomStackIndex) || 
                        (nTempLemmaVble == nBacktrackAtom))
                  {
                     arrTempLemma[nCopytoIndex++] = nTempLemmaLiteral;
                     nLemmaScore += arrBacktrackStackIndex[nTempLemmaVble];
                  }
                  else
                     arrUnsetLemmaFlagVars[nUnsetLemmaFlagIndex++] 
                        = nTempLemmaVble;
               }
               nTempLemmaIndex = nCopytoIndex;

               if (nTempLemmaIndex <= 0)
               {
                  // Lemma of length zero. The problem is unsat.
                  cout << "1: Lemma of length zero." << endl;
                  return 0; // goto_NoSolution;
               }

               /* COMPUTE nLemmaScore where lemmas with lower level literal 
                * are WORSE!!! 
                */ 

               bool bFlag = false;
               if (MAX_NUM_CACHED_LEMMAS) bFlag = true;
               /*
                if (MAX_NUM_CACHED_LEMMAS && nBacktrackStackIndex > 0)
                {
                double fLemmaScore = 2.0 * nLemmaScore/
                (nBacktrackStackIndex*(nBacktrackStackIndex+1));

                if (fLemmaScore < LEMMA_CONSTANT) bFlag = true;
                }
                */

               assert(IsInLemmaList(pUnitLemmaListTail,
                        pUnitLemmaList));	  

               pNewLemma=AddLemma(nTempLemmaIndex,
                     arrTempLemma,
                     bFlag,
                     pUnitLemmaList, /*m lemma is added in here */
                     pUnitLemmaListTail /*m and here */
                     );

            }

            /*m while (1) - the end of backtrack */
            if(nBacktrackAtom == nInferredAtom) break; 


            if(pBacktrackTop->pUnitLemmaList != NULL) 
            {
               /*m join pUnitLemmaList with the pBacktrackTop->pUnitLemmaList */
               assert(IsInLemmaList(pBacktrackTop->pUnitLemmaListTail,
                        pBacktrackTop->pUnitLemmaList));
               pBacktrackTop->pUnitLemmaListTail->pNextLemma[0]
                  = pUnitLemmaList->pNextLemma[0];
               pUnitLemmaList->pNextLemma[0] = pBacktrackTop->pUnitLemmaList;
               if(pUnitLemmaListTail == NULL) 
                  pUnitLemmaListTail = pBacktrackTop->pUnitLemmaListTail;
               assert(IsInLemmaList(pUnitLemmaListTail, pUnitLemmaList));
            }

         } // if(pBacktrackTop->bWasChoicePoint == true || ... )

#ifdef DISPLAY_TRACE
         if (nNumBacktracks >= TRACE_START)
         {
            cout << "Examining lemma:" << endl;
            DisplayLemmaStatus(pBacktrackTop->pLemma, arrSolution);
            cout << "which witnesses inference X"
               << pBacktrackTop->nAtom << " = "
               << arrSolution[pBacktrackTop->nAtom]
               << endl;
         }
#endif

         // Check whether the atom at the top of the backtrack stack
         // is relevent to the resolution done so far.
         if (arrLemmaFlag[nBacktrackAtom])
         {
            // The backtrack atom is relevant to the resolution done so far.
            // Therefore, include its attached lemma in the resolution process.
# ifdef DISPLAY_TRACE
            if (nNumBacktracks >= TRACE_START)
            {
               cout << "Lemma relevant to contradiction." << endl;
            }
# endif


            //m copy all literal not marked in arrLemmaFlag into arrTempLemma
            LemmaBlock *pLemmaBlock = pBacktrackTop->pLemma;
            int *arrLits = pLemmaBlock->arrLits;
            int nLemmaLength = arrLits[0];
            int nLemmaLiteral;
            int nLemmaVble;

            for (int nLitIndex = 1, nLitIndexInBlock = 1;
                  nLitIndex <= nLemmaLength;
                  nLitIndex++, nLitIndexInBlock++)
            {
               if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
               {
                  nLitIndexInBlock = 0;
                  pLemmaBlock = pLemmaBlock->pNext;
                  arrLits = pLemmaBlock->arrLits;
               }
               nLemmaLiteral = arrLits[nLitIndexInBlock];
               nLemmaVble = abs(nLemmaLiteral);
               if (arrLemmaFlag[nLemmaVble] == false)
               {
                  arrLemmaFlag[nLemmaVble] = true;
                  arrTempLemma[nTempLemmaIndex++] = nLemmaLiteral;
                  if(arrBacktrackStackIndex[nLemmaVble] > arrBacktrackStackIndex[nInferredAtom]) nNumForcedInfsBelowCurrentCP++;
                  assert(arrSolution[nLemmaVble] 
                        == (nLemmaLiteral > 0 ? BOOL_FALSE : BOOL_TRUE));
               }
            }
         }
#ifdef DISPLAY_TRACE
         else if (nNumBacktracks >= TRACE_START)
         {
            cout << "Lemma irrelevant to contradiction." << endl;
         }
         cout << "Star_Count = " << nNumForcedInfsBelowCurrentCP << endl;

         if (nNumBacktracks >= TRACE_START)
         {
            cout << "Backtracking from forced assignment of X"
               << nBacktrackAtom << " equal to "
               << (arrSolution[nBacktrackAtom] == BOOL_TRUE ? "true" : "false")
               << "." << endl;
         }
#endif

         if (pBacktrackTop->pLemmaInfo) {
            FreeLemma(pBacktrackTop->pLemmaInfo, false);
         }

         arrSolution[nBacktrackAtom] = BOOL_UNKNOWN;
      };  //  while (1)

      arrSolution[nInferredAtom] = BOOL_UNKNOWN;
      nNumBackjumps++;

   }  // backjumping loop
   while (arrLemmaFlag[nInferredAtom] == false);

   nNumBackjumps--; /*m last backjump was not just backtrack */

   /*m clean up */
   for(int i = 0; i < nUnsetLemmaFlagIndex; i++)
      arrLemmaFlag[arrUnsetLemmaFlagVars[i]] = false;
   for(int i = 0; i < nTempLemmaIndex; i++) 
      arrLemmaFlag[abs(arrTempLemma[i])] = false;

   BacktrackStackEntry *pBacktrackStackOldBranchPoint = pBacktrackTop;

   // Flush the inference queue.
   pInferenceQueueNextElt = pInferenceQueueNextEmpty = arrInferenceQueue;
   pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;

   // Reverse the polarity of the branch atom.
   nInferredValue = (nInferredValue == BOOL_TRUE ? BOOL_FALSE : BOOL_TRUE);

   // Mark the stacks for autarky use
   //Mark_arrSmurfStatesStack();
   //Mark_arrNumRHSUnknowns();

   InferLiteral(nInferredAtom, nInferredValue, true, pNewLemma, NULL, 1);

#ifdef DISPLAY_TRACE
   if (nNumBacktracks >= TRACE_START)
   {
      cout << "Reversed polarity of X" << nInferredAtom
         << " to " << nInferredValue << endl;
      //DisplayAllBrancherLemmas();
      cout << "Would DisplayAllBrancherLemmas here." << endl;
   }
#endif  

   // Add inferences from lemmas which became unit through the course of
   // the backtracking.
   int nInferredAtomLevel = arrBacktrackStackIndex[nInferredAtom];

   // We have to skip past the most recently added lemma because 
   // it witnesses that we have to reverse polarity, but we have 
   // already done so.
   LemmaInfoStruct *previous = pUnitLemmaList->pNextLemma[0];
   for (LemmaInfoStruct *p = previous->pNextLemma[0]; p; p = p->pNextLemma[0])
   {
      //m WatchedVble 1 and 2 have the highest BSI in the whole clause
      //m i.e. if Watched2 is < InferredAtomLevel => Watched1 is inferred
      if(arrBacktrackStackIndex[p->nWatchedVble[1]] >= nInferredAtomLevel || p->nWatchedVble[0] == 0) 
      {
         //Remove lemma p from UnitLemmaList and add it into the lemma cache
         previous->pNextLemma[0] = p->pNextLemma[0];
         if (p == pUnitLemmaListTail) pUnitLemmaListTail = previous;

         AddLemmaIntoCache(p);

         p = previous;

         continue;
      }
      else
      {
         //It is the case that no two lemmas in the UnitLemmaList
         //will ever apply the same inference. Thus nWatchedVble[0] always
         //is the index of a variable which has not yet been branched on.
         assert(p->nWatchedVble[1] != nInferredAtom);

         //Apply the inference to nWatchedVble[0]

         assert(arrSolution[p->nWatchedVble[0]] == BOOL_UNKNOWN);

         InferLiteral(p->nWatchedVble[0], p->nWatchedVblePolarity[0],
               false, p->pLemma, NULL, 1);

         previous = previous->pNextLemma[0];
         continue; 
      }
   } // for (LemmaInfoStruct *p = pUnitLemmaList->pNextLemma[0]; ; p = p->pNextLemma[0])

   //cout << "Placing unit lemma list on the stack:" << endl;
   //DisplayLemmaList(pUnitLemmaList);
   pBacktrackStackOldBranchPoint->pUnitLemmaList = pUnitLemmaList->pNextLemma[0];
   pBacktrackStackOldBranchPoint->pUnitLemmaListTail = pUnitLemmaListTail;

   pUnitLemmaList->pNextLemma[0] = NULL;
   pUnitLemmaListTail = NULL;

   // Get the consequences of the branch atoms new value.
   return 1;
}

ITE_INLINE
int
ConstructTempLemma()
{
   int nTempLemmaIndex=0;
   LemmaBlock *pConflictLemmaBlock = pConflictLemma;
   int *arrConflictLemmaLits = pConflictLemmaBlock->arrLits;
   int nConflictLemmaLength = arrConflictLemmaLits[0];

   for (int nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nConflictLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pConflictLemmaBlock = pConflictLemmaBlock->pNext;
         arrConflictLemmaLits = pConflictLemmaBlock->arrLits;
      }
      int nConflictLiteral = arrConflictLemmaLits[nLitIndexInBlock];
      int nConflictVble = abs(nConflictLiteral);
      assert(arrSolution[nConflictVble] != BOOL_UNKNOWN);
      assert(arrSolution[nConflictVble]
            == (nConflictLiteral > 0 ? BOOL_FALSE : BOOL_TRUE));
      assert(!arrLemmaFlag[nConflictVble]);
      arrTempLemma[nTempLemmaIndex++] = nConflictLiteral;
      arrLemmaFlag[nConflictVble] = true;
   }
   if (pConflictLemmaInfo) {
      FreeLemma(pConflictLemmaInfo, false);
      pConflictLemmaInfo = NULL;
   }
   return nTempLemmaIndex;
}


ITE_INLINE
void
InferLiteral(int nInferredAtom,
      int nInferredValue,
      bool bWasChoicePoint,
      LemmaBlock *pLemma,
      LemmaInfoStruct *pCachedLemma,
      int infer)
{
   assert(nInferredValue == BOOL_TRUE || nInferredValue == BOOL_FALSE);
   D_9(
         if (nNumBacktracks >= TRACE_START)
         {
         d9_printf3("Inferring %c%d ", (nInferredValue==BOOL_TRUE?'+':'-'),
            nInferredAtom);
         if (pLemma) {
         DisplayLemmaStatus(pLemma);
         } else {
         if (infer == 0)
         d9_printf1("(choice point)\n");
         }
         }
      )

      /*
       * flag changed structures
       */
      AffectedFuncsStruct *pAFS = arrAFS + nInferredAtom;

   /* Flag all smurfs */
   // Determine the potentially affected regular Smurfs.
   IndexRoleStruct *pIRS = pAFS->arrSpecFuncsAffected;
   int nNumRegSmurfsAffected = pAFS->nNumRegSmurfsAffected;
   int *arrRegSmurfsAffected = pAFS->arrRegSmurfsAffected;
   int nNumSpecialFuncsAffected = pAFS->nNumSpecialFuncsAffected;

   // Update each affected regular Smurf.
   for (int i = 0; i < nNumRegSmurfsAffected; i++)
   {
      if (arrCurrentStates[arrRegSmurfsAffected[i]] == pTrueSmurfState) continue;
      if ((arrChangedSmurfs[arrRegSmurfsAffected[i]]&1) != 0) continue;
      if (arrChangedSmurfs[arrRegSmurfsAffected[i]] == 0) {
            /* add on the heuristic update stack */
            Save_arrCurrentStates(arrRegSmurfsAffected[i]);
            assert(pFnInferenceQueueNextElt - arrFnInferenceQueue < (nNumRegSmurfs+nNumSpecialFuncs));
            pFnInferenceQueueNextElt->fn_type = 0;
            pFnInferenceQueueNextElt->fn_id = arrRegSmurfsAffected[i];
            pFnInferenceQueueNextElt++;
      }
      arrChangedSmurfs[arrRegSmurfsAffected[i]]=3;
      // FIXME: if there are no inferences -> transition rightaway and don't flag it
      // FIXME: NEW: add to inferenced function stack (SMURF, i)
      //Save_arrCurrentStates(arrRegSmurfsAffected[i]);
   }

   /* Flag all specfns */
   // Determine the potentially affected special functions.

   // Update each affected special function.
   for (int i = 0; i < nNumSpecialFuncsAffected; i++, pIRS++)
   {
      if (arrNumRHSUnknowns[pIRS->nSpecFuncIndex] == 0) continue;

      int nSpecFuncIndex = pIRS->nSpecFuncIndex;

       
       //if (arrSpecialFuncs[nSpecFuncIndex].nFunctionType == XOR && 
       //arrNumRHSUnknownsNew[nSpecFuncIndex] > 2) {
       //Save_arrNumRHSUnknowns(nSpecFuncIndex);
       //arrNumRHSUnknowns[nSpecFuncIndex] = --arrNumRHSUnknownsNew[nSpecFuncIndex];
       //continue;
       //}
       
      if ((arrChangedSpecialFn[nSpecFuncIndex]&1)==0) {
         if (arrChangedSpecialFn[nSpecFuncIndex]==0) {
            /* add on the heuristic update stack */
            Save_arrNumRHSUnknowns(nSpecFuncIndex);
            assert(pFnInferenceQueueNextElt - arrFnInferenceQueue < (nNumRegSmurfs+nNumSpecialFuncs));
            pFnInferenceQueueNextElt->fn_type = 1;
            pFnInferenceQueueNextElt->fn_id = nSpecFuncIndex;
            pFnInferenceQueueNextElt++;
         }
         arrChangedSpecialFn[nSpecFuncIndex]=3;
      }
      // FIXME: if there are no inferences -> transition rightaway and don't flag it
      //
      // arrSpecialFuncs[nSpecFuncIndex].nFunctionType
      //Save_arrNumRHSUnknowns(nSpecFuncIndex);
      if (pIRS->nRHSIndex >= 0) arrNumRHSUnknownsNew[nSpecFuncIndex]--;
      else arrNumLHSUnknownsNew[nSpecFuncIndex]--;

      // FIXME: NEW: add to inferenced function stack (SPFN, i)
   }
   /*
    * End of flagging
    */

   assert (arrSolution[nInferredAtom] == BOOL_UNKNOWN);
   arrSolution[nInferredAtom] = nInferredValue;

   // Enqueue inference.
   *(pInferenceQueueNextEmpty++) = nInferredAtom;

   arrBacktrackStackIndex[nInferredAtom] = nBacktrackStackIndex++;

   pBacktrackTop->nAtom = nInferredAtom;
   pBacktrackTop->bWasChoicePoint = bWasChoicePoint;
   pBacktrackTop->pLemma = pLemma;
   pBacktrackTop->pLemmaInfo = pCachedLemma;
   pBacktrackTop++;
}

ITE_INLINE void
InferNLits(SpecialFunc *pSpecialFunc, int n)
{
   int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
   int *arrElts = pSpecialFunc->rhsVbles.arrElts;
   for (int j = 0; j < nNumElts && n; j++)
   {
      if (arrSolution[arrElts[j]] == BOOL_UNKNOWN)
      {
         n--;
         InferLiteral(arrElts[j], pSpecialFunc->arrRHSPolarities[j],
               false,
               pSpecialFunc->arrShortLemmas[j],
               NULL, 1);
      }
   }
   assert(n==0);
}


ITE_INLINE
void
AddLemmaIntoCache(LemmaInfoStruct *p)
{
   if(p->bPutInCache) //Add p to the two corresponding lemma lists...
   {
      /* the only place to increase gnNumCachedLemmas */
      if (++gnNumCachedLemmas > MAX_NUM_CACHED_LEMMAS) FreeLemmas(1);

#ifdef HEURISTIC_USES_LEMMA_COUNTS
      p->bIsInCache = true;
#endif
      //Add to watched literal 1s lemma list
      AffectedFuncsStruct *pAFS = arrAFS + p->nWatchedVble[0];
      if(p->nWatchedVblePolarity[0])
      {
         p->pNextLemma[0] = pAFS->LemmasWherePos[0].pNextLemma[0];
         p->pPrevLemma[0] = &(pAFS->LemmasWherePos[0]);
         pAFS->LemmasWherePos[0].pNextLemma[0] = p;
         if (p->pNextLemma[0]) p->pNextLemma[0]->pPrevLemma[0] = p;
      }
      else
      {
         p->pNextLemma[0] = pAFS->LemmasWhereNeg[0].pNextLemma[0];
         p->pPrevLemma[0] = &(pAFS->LemmasWhereNeg[0]);
         pAFS->LemmasWhereNeg[0].pNextLemma[0] = p;
         if (p->pNextLemma[0]) p->pNextLemma[0]->pPrevLemma[0] = p;
      }

      //Add to watched literal 2s lemma list
      pAFS = arrAFS + p->nWatchedVble[1];
      if(p->nWatchedVblePolarity[1])
      {
         p->pNextLemma[1] = pAFS->LemmasWherePos[1].pNextLemma[1];
         p->pPrevLemma[1] = &(pAFS->LemmasWherePos[1]);
         pAFS->LemmasWherePos[1].pNextLemma[1] = p;
         if (p->pNextLemma[1]) p->pNextLemma[1]->pPrevLemma[1] = p;
      }
      else
      {
         p->pNextLemma[1] = pAFS->LemmasWhereNeg[1].pNextLemma[1];
         p->pPrevLemma[1] = &(pAFS->LemmasWhereNeg[1]);
         pAFS->LemmasWhereNeg[1].pNextLemma[1] = p;
         if (p->pNextLemma[1]) p->pNextLemma[1]->pPrevLemma[1] = p;
      }

      LPQEnqueue(p);
   }
   else
   {
      // The lemma does not go into the lemma cache.
      // Recycle the lemma immediately.
      FreeLemma(p, false);
   }
}

