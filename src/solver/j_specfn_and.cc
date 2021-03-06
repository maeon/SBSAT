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

#include "ite.h"
#include "solver.h"

struct AndEqFalseWghtStruct *arrAndEqFalseWght = NULL;
struct AndEqWghtStruct *arrAndEqWght = NULL;
double *arrAndEqWghtCx = NULL;
double *arrAndEqWghtCe = NULL;
double *arrAndEqWghtCt = NULL; // lhs is false

ITE_INLINE void InitHeuristicTablesForSpecialFuncs_AND();
ITE_INLINE void FreeHeuristicTablesForSpecialFuncs_AND();

ITE_INLINE
void
InitHeuristicTablesForSpecialFuncs_AND()
   // Initialize some tables for computing heuristic values
   // for variables that are mentioned in special functions.
{

   HWEIGHT K = JHEURISTIC_K;

   // We need nMaxRHSSize to be at least one to insure that entry 1 exists
   // and we don't overrun the arrays.
   int nMaxRHSSize = 1;
   if (arrSpecialFuncs)
      for(int i=0; arrSpecialFuncs[i].nFunctionType != 0; i++) 
         if (arrSpecialFuncs[i].nFunctionType == SFN_AND)
            if (nMaxRHSSize < arrSpecialFuncs[i].rhsVbles.nNumElts)
               nMaxRHSSize = arrSpecialFuncs[i].rhsVbles.nNumElts;

   arrAndEqFalseWght = (AndEqFalseWghtStruct*)ite_calloc(nMaxRHSSize + 1, sizeof(AndEqFalseWghtStruct), 
          9, "arrAndEqFalseWght");

   arrAndEqWght = (AndEqWghtStruct*)ite_calloc(nMaxRHSSize + 1,  sizeof(AndEqWghtStruct),
          9, "arrAndEqWght");

   arrAndEqWghtCx = (double*)ite_calloc(nMaxRHSSize + 2, sizeof(double), 9, "arrAndEqWghtCx");
   arrAndEqWghtCe = (double*)ite_calloc(nMaxRHSSize + 2, sizeof(double), 9, "arrAndEqWghtCe");
   arrAndEqWghtCt = (double*)ite_calloc(nMaxRHSSize + 2, sizeof(double), 9, "arrAndEqWghtCt");

   if (sHeuristic[1] == 'm') {

      for (int i = 1; i <= nMaxRHSSize; i++)
      {
         arrAndEqFalseWght[i].fPos = 1.0 / i+1;
         arrAndEqFalseWght[i].fFmla = 1.0 / i+1;
         arrAndEqWght[i].fLHSPos = 1.0 / i+1;
         arrAndEqWght[i].fLHSNeg = 1.0 / i+1;
         arrAndEqWght[i].fRHSPos = 1.0 / i+1;
         arrAndEqWght[i].fRHSNeg = 1.0 / i+1;
         arrAndEqWght[i].fFmla   = 1.0 / i+1;
      }
   } else {
      arrAndEqFalseWght[1].fPos = 2*JHEURISTIC_K_TRUE+1*JHEURISTIC_K_INF;
      arrAndEqFalseWght[1].fFmla = arrAndEqFalseWght[1].fPos/(2*K);

      arrAndEqWghtCt[1] = 1;
      arrAndEqWghtCx[1] = 1/(2*K);
      arrAndEqWghtCe[1] = 1/(2*K);

      for (int i = 2; i <= nMaxRHSSize; i++)
      {
         arrAndEqFalseWght[i].fPos = JHEURISTIC_K_TRUE+arrAndEqFalseWght[i - 1].fFmla;
         arrAndEqFalseWght[i].fFmla = arrAndEqFalseWght[i].fPos / (2 * K);
         arrAndEqWghtCt[i] =  arrAndEqWghtCt[i-1]*(i-1)/(2*i*K);
         arrAndEqWghtCx[i] = (arrAndEqWghtCx[i-1] + 1) * i / (2*K*(i+1));
         arrAndEqWghtCe[i] = (arrAndEqWghtCe[i-1]*(i-1) + arrAndEqWghtCt[i] + 1) / (2*K*(i+1));
      }

      // move it all one up
      for (int i = nMaxRHSSize+1; i> 0; i--)
      {
         arrAndEqWghtCt[i] = arrAndEqWghtCt[i-1];
         arrAndEqWghtCx[i] = arrAndEqWghtCx[i-1];
         arrAndEqWghtCe[i] = arrAndEqWghtCe[i-1];
      }
      arrAndEqWghtCx[1] = 1;
      arrAndEqWghtCe[1] = 0;
      arrAndEqWghtCt[1] = 0;

      for (int i = 1; i <= nMaxRHSSize; i++)
      {
         arrAndEqWght[i].fLHSPos = i*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE;
         arrAndEqWght[i].fLHSNeg = arrAndEqFalseWght[i].fFmla;
         arrAndEqWght[i].fRHSPos = (i == 1 ? (1*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE) : 
               arrAndEqWght[i - 1].fFmla);
         arrAndEqWght[i].fRHSNeg = 1*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE;
         arrAndEqWght[i].fFmla
            = (arrAndEqWght[i].fLHSPos
                  + arrAndEqWght[i].fLHSNeg
                  + i * arrAndEqWght[i].fRHSPos
                  + i * arrAndEqWght[i].fRHSNeg) / (2 * (i + 1) * K);
      }
   }
}

ITE_INLINE void
FreeHeuristicTablesForSpecialFuncs_AND() 
{
   ite_free((void**)&arrAndEqFalseWght);
   ite_free((void**)&arrAndEqWght);
   ite_free((void**)&arrAndEqWghtCt);
   ite_free((void**)&arrAndEqWghtCx);
   ite_free((void**)&arrAndEqWghtCe);
}


ITE_INLINE void
GetHeurScoresFromSpecialFunc_AND(int nSpecFuncIndex)
   // Used in initialization of heuristic scores.
   // For the special function with index nSpecFuncIndex,
   // determine all of the uninstantiated variables.
   // Determine the weight which the special function contributes
   // to the positive and negative scores for each such variable.
   // Add these weights into the corresponding arrays.
{
   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;
   int nNumRHSUnknowns = arrNumRHSUnknowns[nSpecFuncIndex];
   int nLHSVble = pSpecialFunc->nLHSVble;
   int nLHSVbleValue = arrSolution[nLHSVble];
   int nLHSPolarity = pSpecialFunc->nLHSPolarity;
   int nLHSLitValue = BOOL_UNKNOWN;


   if (nLHSVbleValue != BOOL_UNKNOWN)
   {
      nLHSLitValue = nLHSVbleValue == nLHSPolarity ? 
         BOOL_TRUE : BOOL_FALSE;
   }

   if (nLHSLitValue == BOOL_TRUE)
   {
      // We should have closed the inference set before calling this routine.
      assert (nNumRHSUnknowns <= 0);
      return;
   }
   else if (nLHSLitValue == BOOL_FALSE)
   {
      HWEIGHT fPosDelta = arrAndEqFalseWght[nNumRHSUnknowns].fPos;
      HWEIGHT fNegDelta = arrAndEqFalseWght[nNumRHSUnknowns].fNeg; // 0

      J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
   }
   else
   {
      // LHS literal is uninstantiated.

      Save_arrHeurScores(nLHSVble);

      // Handle scores for LHS variable.
      if (nLHSPolarity == BOOL_TRUE)
      {
         arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSPos;
         arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSNeg;
      }
      else
      {
         arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSNeg;
         arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSPos;
      }

      // Handle scores for RHS variables.
      HWEIGHT fPosDelta = arrAndEqWght[nNumRHSUnknowns].fRHSPos;
      HWEIGHT fNegDelta = arrAndEqWght[nNumRHSUnknowns].fRHSNeg;

      J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
   }
}

ITE_INLINE void
GetHeurScoresFromSpecialFunc_AND_C(int nSpecFuncIndex)
   // Used in initialization of heuristic scores.
   // For the special function with index nSpecFuncIndex,
   // determine all of the uninstantiated variables.
   // Determine the weight which the special function contributes
   // to the positive and negative scores for each such variable.
   // Add these weights into the corresponding arrays.
{
   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;
   int nNumRHSUnknowns = arrNumRHSUnknowns[nSpecFuncIndex];
   int nLHSVble = pSpecialFunc->nLHSVble;
   int nLHSVbleValue = arrSolution[nLHSVble];
   int nLHSPolarity = pSpecialFunc->nLHSPolarity;
   int nLHSLitValue = BOOL_UNKNOWN;
   double fSum = arrSumRHSUnknowns[nSpecFuncIndex];

   if (nLHSVbleValue != BOOL_UNKNOWN)
   {
      nLHSLitValue = nLHSVbleValue == nLHSPolarity ? 
         BOOL_TRUE : BOOL_FALSE;
   }

   if (nLHSLitValue == BOOL_TRUE)
   {
      // We should have closed the inference set before calling this routine.
      assert (nNumRHSUnknowns <= 0);
      return;
   }
   else if (nLHSLitValue == BOOL_FALSE)
   {
      J_Update_RHS_AND_C(pSpecialFunc,
            0, 0, 0, 0, 0, // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fSum, 0, arrAndEqWghtCt[nNumRHSUnknowns/*-1*/], 0, 0); // Ct
   }
   else
   {
      // LHS literal is uninstantiated.

      Save_arrHeurScores(nLHSVble);

      // Handle scores for LHS variable.
      if (nLHSPolarity == BOOL_TRUE)
      {
         arrHeurScores[nLHSVble].Pos += fSum; // the RHS is inferred
         arrHeurScores[nLHSVble].Neg += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSum; // Transition to Ct
      }
      else
      {
         arrHeurScores[nLHSVble].Pos += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSum; // Transition to Ct
         arrHeurScores[nLHSVble].Neg += fSum; // the RHS is inferred
      }

      J_Update_RHS_AND_C(pSpecialFunc,
            0, 0, 0, 0, 0, // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fSum, arrAndEqWghtCx[nNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], arrAndEqWghtCe[nNumRHSUnknowns/*-1*/], 
            arrJWeights[nLHSVble], 0);
   }
}


ITE_INLINE void
J_UpdateHeuristic_AND(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      int nOldNumLHSUnknowns, int nNumLHSUnknowns)
{
   HWEIGHT fPosDelta;
   HWEIGHT fNegDelta;

   /* remove heuristic value contribution of this constrains */

   /* figure out if LHS was inferred before this lit
    * in order to subtract its Heu value 
    */
   if (nOldNumLHSUnknowns == 0)
   {
      fPosDelta = arrAndEqFalseWght[nOldNumRHSUnknowns].fPos * -1;
      fNegDelta = arrAndEqFalseWght[nOldNumRHSUnknowns].fNeg * -1;
      fPosDelta += arrAndEqFalseWght[nNumRHSUnknowns].fPos;
      fNegDelta += arrAndEqFalseWght[nNumRHSUnknowns].fNeg;
      J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
      return;
   }

   // Handle scores for RHS variables.
   fPosDelta = arrAndEqWght[nOldNumRHSUnknowns].fRHSPos * -1;
   fNegDelta = arrAndEqWght[nOldNumRHSUnknowns].fRHSNeg * -1;

   if (nNumLHSUnknowns == 0)
   {
      fPosDelta += arrAndEqFalseWght[nNumRHSUnknowns].fPos;
      fNegDelta += arrAndEqFalseWght[nNumRHSUnknowns].fNeg;
      J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
      return;
   }

   // prev LHS literal was uninstantiated.
   int nLHSVble = pSpecialFunc->nLHSVble;
   assert(nLHSVble > 0);

   Save_arrHeurScores(nLHSVble);

   // Handle scores for LHS variable.
   if (pSpecialFunc->nLHSPolarity == BOOL_TRUE)
   {
      arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSPos -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSPos;
      arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSNeg -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSNeg;
   }
   else
   {
      arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSNeg -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSNeg;
      arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSPos -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSPos;
   }

//#define MK_BETTER_QUESTION
#ifdef MK_BETTER_QUESTION
   J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
   fPosDelta = 0;
   fNegDelta = 0;
#endif

   fPosDelta += arrAndEqWght[nNumRHSUnknowns].fRHSPos;
   fNegDelta += arrAndEqWght[nNumRHSUnknowns].fRHSNeg;

   J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
}

ITE_INLINE void
J_UpdateHeuristic_AND_C(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      int nOldNumLHSUnknowns, int nNumLHSUnknowns,
      double fOldSumRHSUnknowns, double fSumRHSUnknowns)
{
   int nLHSVble = pSpecialFunc->nLHSVble;

   /* remove heuristic value contribution of this constrains */

   /* figure out if LHS was inferred before this lit
    * in order to subtract its Heu value 
    */
   if (nOldNumLHSUnknowns == 0)
   {
      J_Update_RHS_AND_C(pSpecialFunc,
            // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fOldSumRHSUnknowns, 0, arrAndEqWghtCt[nOldNumRHSUnknowns/*-1*/], 0, 0,
            fSumRHSUnknowns, 0, arrAndEqWghtCt[nNumRHSUnknowns/*-1*/], 0, 0);
      return;
   }

   // Handle scores for RHS variables.

   if (nNumLHSUnknowns == 0)
   {
      J_Update_RHS_AND_C(pSpecialFunc,
            // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fOldSumRHSUnknowns, arrAndEqWghtCx[nOldNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], 
            arrAndEqWghtCe[nOldNumRHSUnknowns/*-1*/], arrJWeights[nLHSVble], 0,
            fSumRHSUnknowns, 0, arrAndEqWghtCt[nNumRHSUnknowns/*-1*/], 0, 0);

      return;
   }

   // prev LHS literal was uninstantiated.
   assert(nLHSVble > 0);

   Save_arrHeurScores(nLHSVble);

   // Handle scores for LHS variable.
   if (pSpecialFunc->nLHSPolarity == BOOL_TRUE)
   {
      arrHeurScores[nLHSVble].Pos += fSumRHSUnknowns - fOldSumRHSUnknowns; // the RHS is inferred
      arrHeurScores[nLHSVble].Neg += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSumRHSUnknowns -
                                    arrAndEqWghtCt[nOldNumRHSUnknowns+1]*fOldSumRHSUnknowns; // Transition to Ct
   }
   else
   {
      arrHeurScores[nLHSVble].Pos += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSumRHSUnknowns -
                                    arrAndEqWghtCt[nOldNumRHSUnknowns+1]*fOldSumRHSUnknowns; // Transition to Ct
      arrHeurScores[nLHSVble].Neg += fSumRHSUnknowns - fOldSumRHSUnknowns; // the RHS is inferred
   }
   J_Update_RHS_AND_C(pSpecialFunc,
         // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fOldSumRHSUnknowns, arrAndEqWghtCx[nOldNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], 
            arrAndEqWghtCe[nOldNumRHSUnknowns/*-1*/], arrJWeights[nLHSVble], 0,
            fSumRHSUnknowns, arrAndEqWghtCx[nNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], 
            arrAndEqWghtCe[nNumRHSUnknowns/*-1*/], arrJWeights[nLHSVble], 0);

}

