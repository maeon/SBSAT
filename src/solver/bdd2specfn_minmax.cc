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

ITE_INLINE void
BDD2Specfn_MINMAX(BDDNodeStruct *pFunc,
      int nFunctionType,
      int _nEqualityVble,
      SpecialFunc *pSpecialFunc
      )
{
   assert(nFunctionType == MINMAX); // PLAINMINMAX?

   /* -------------- Left Hand Side -------------- */

   // the result should be ODD for fn to be sat
   // The function is of the form 1 = l_1 + ... + l_n.
   // LHS should be -FALSE ==> TRUE
   // Store LHS variable. and polarity
   pSpecialFunc->nLHSVble = arrIte2SolverVarMap[0]; /* ALWAYS FALSE VARIABLE */
   pSpecialFunc->nLHSPolarity = BOOL_FALSE;
   pSpecialFunc->nFunctionType = SFN_MINMAX;

   /* -------------- Right Hand Side -------------- */
   /* 0. Create the List of Variables and Polarities */		  

   // Store variable set of RHS.
   // Here we assume that the LHS variable does not also occur on
   // the RHS of the equality.
   long tempint_max = 0;
   long y=0;
   unravelBDD(&y, &tempint_max, &pSpecialFunc->rhsVbles.arrElts, pFunc);
   qsort(pSpecialFunc->rhsVbles.arrElts, y, sizeof(int), revcompfunc);
   pSpecialFunc->rhsVbles.nNumElts = y;
   pSpecialFunc->rhsVbles.arrElts = (int*)realloc(pSpecialFunc->rhsVbles.arrElts, pSpecialFunc->rhsVbles.nNumElts*sizeof(int));

   // Determine the polarities of the literals on the RHS of the equation:
   pSpecialFunc->arrRHSPolarities
      = (int *)ite_calloc(pSpecialFunc->rhsVbles.nNumElts, sizeof(int),
            9, "special function polarities");
   int *arrRHSPolarities = pSpecialFunc->arrRHSPolarities;

   /* set polarities and map variables from ite to solver ordering */
   for (int i = 0; i < pSpecialFunc->rhsVbles.nNumElts; i++) {
      pSpecialFunc->rhsVbles.arrElts[i] = 
         arrIte2SolverVarMap[pSpecialFunc->rhsVbles.arrElts[i]];
      arrRHSPolarities[i] = BOOL_UNKNOWN;
   }

   /* 1. Create BDD representing RHS. */
   BDDNodeStruct *pRHSFunc = pFunc;

   /* 2. Traverse the BDD, storing the variable index and polarity  for each literal. */
   BDDNodeStruct *pCurrentNode = pRHSFunc;
   int i = 0; // Index into the array of variables mentioned in the func.
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      if (pSpecialFunc->rhsVbles.arrElts[i] != arrIte2SolverVarMap[pCurrentNode->variable]) {
         printBDD(pRHSFunc);
         assert(0);
      }
      assert(pSpecialFunc->rhsVbles.arrElts[i] == arrIte2SolverVarMap[pCurrentNode->variable]);
      arrRHSPolarities[i] = BOOL_TRUE;
      if (pCurrentNode->thenCase != true_ptr && pCurrentNode->thenCase != false_ptr)
         pCurrentNode = pCurrentNode->thenCase;
      else
         pCurrentNode = pCurrentNode->elseCase;
      i++;
   }

   pSpecialFunc->max=0;
   pCurrentNode = pRHSFunc;
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      pCurrentNode = pCurrentNode->thenCase;
      pSpecialFunc->max++;
   }
   if (pCurrentNode == true_ptr) pSpecialFunc->max = pSpecialFunc->rhsVbles.nNumElts;
   else pSpecialFunc->max--;

   pSpecialFunc->min=pSpecialFunc->rhsVbles.nNumElts;
   pCurrentNode = pRHSFunc;
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      pCurrentNode = pCurrentNode->elseCase;
      pSpecialFunc->min--;
   }
   if (pCurrentNode == true_ptr) pSpecialFunc->min = 0;
   else pSpecialFunc->min++;

   assert(pSpecialFunc->min <= pSpecialFunc->max);

   ConstructLemmasForMINMAX(pSpecialFunc);
}

