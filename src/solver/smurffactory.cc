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

// External variables.
SpecialFunc *arrSpecialFuncs =  0;
SmurfState **arrRegSmurfInitialStates = 0;
t_smurf_path *arrSmurfPath;
t_smurf_chain *arrSmurfChain;
int *arrSmurfEqualityVble;
int nNumSpecialFuncs;
int nNumRegSmurfs; // Number of regular Smurfs.
int nSpecialFuncIndex;
int nRegSmurfIndex;
int gnMaxVbleIndex;

ITE_INLINE void DisplaySmurfStates();
ITE_INLINE char * StringFromFunctionType(int nFuncType);
ITE_INLINE void InitSmurfFactory();


ITE_INLINE int
SmurfFactory()
{
   double fStartTime = get_runtime();
   int nSumVarsPerSmurf = 0;

   InitSmurfFactory();

   d9_printf1("SmurfFactory\n");

   int *arrIte2SolverSpecFn = (int*)ite_calloc(nmbrFunctions, sizeof(int), 
         9, "arrIte2SolverSpecFn");
   int *arrIte2SolverSmurf = (int*)ite_calloc(nmbrFunctions, sizeof(int),
         9, "arrIte2SolverSmurf");
   for (int i = 0; i<nmbrFunctions; i++) {
      arrIte2SolverSpecFn[i] = -1;
      arrIte2SolverSmurf[i] = -1;
   }

   // Construct the Smurfs.
   for (int i = 0; i < nmbrFunctions; i++)
   {
      int nFunctionType = functionType[i];
      BDDNodeStruct *pFunc = functions[i];
      if (pFunc == false_ptr)  return SOLV_UNSAT;
      BDDNode *pSpecFunc = xorFunctions[i];

      if (pSpecFunc != NULL) 
      {
         d4_printf1("&");
         /* spec fn is SFN_XOR! */
      } else {
         if (IsSpecialFunc(nFunctionType))
         {
            pSpecFunc = functions[i];
         }
      }

      d5_printf4("Function %d(%d,%d): ", i, length[i], nFunctionType);
      D_5( printBDD(functions[i]); );
      d5_printf1("\n");

      if (IsSpecialFunc(nFunctionType) == 0)
      {
         assert(pFunc != true_ptr && pFunc != false_ptr);
         //if (i%100 == 0)
           d2e_printf3("\rCreating Smurfs ... %d/%d", i, nmbrFunctions);

			//if(i % 100 == 0)
			  d3_printf3("Constructing Smurf for %d/%d           \r", i, nmbrFunctions);
         arrIte2SolverSmurf[i] = nRegSmurfIndex;
         arrSmurfEqualityVble[nRegSmurfIndex] = arrIte2SolverVarMap[equalityVble[i]];

         SmurfState *pSmurfState = BDD2Smurf(pFunc);

         if (pSmurfState == NULL)
         {
            fprintf(stderr, "Could not create the Smurf state.");
            return SOLV_ERROR;
         }
         nSumVarsPerSmurf += pSmurfState->vbles.nNumElts;
         arrRegSmurfInitialStates[nRegSmurfIndex] = pSmurfState;
         pSmurfState->cVisited |= 1;
         arrSmurfPath[nRegSmurfIndex].literals = 
            (int*)ite_calloc(arrRegSmurfInitialStates[nRegSmurfIndex]->vbles.nNumElts+1, sizeof(int),
               9, "arrSmurfPath[].literals");
         nRegSmurfIndex++;
      } 

      if (pSpecFunc != NULL)
      {
         assert(pSpecFunc != true_ptr && pSpecFunc != false_ptr);
         
         //if (i%100 == 0)
           d2e_printf3("\rCreating Special Function ... %d/%d", i, nmbrFunctions);
         if (IsSpecialFunc(nFunctionType) == 0) {
            nFunctionType = PLAINXOR;
            arrSpecialFuncs[nSpecialFuncIndex].LinkedSmurfs = nRegSmurfIndex-1;
         } else {
            arrSpecialFuncs[nSpecialFuncIndex].LinkedSmurfs = -1;
         }
			//if(i % 100 == 0)
			  d3_printf3("Constructing Special Function for %d/%d\r", i, nmbrFunctions);
         arrIte2SolverSpecFn[i] = nSpecialFuncIndex;
         switch(nFunctionType) {
          case AND: 
          case OR: 
          case PLAINOR: 
             BDD2Specfn_AND(pSpecFunc, nFunctionType,
                   equalityVble[i], &(arrSpecialFuncs[nSpecialFuncIndex]));
             break;
          case PLAINXOR: 
             BDD2Specfn_XOR(pSpecFunc, nFunctionType,
                   equalityVble[i], &(arrSpecialFuncs[nSpecialFuncIndex]));
             break;
          case MINMAX:
             BDD2Specfn_MINMAX(pSpecFunc, nFunctionType,
                   equalityVble[i], &(arrSpecialFuncs[nSpecialFuncIndex]));
             break;
          default: assert(0); exit(1);
         }

         nSpecialFuncIndex++;
      }

#ifdef PRINT_SMURF_STATS
      //printf("Constructed Smurf: %d  Total Smurf states: %d Func Type: %d",
      //i, nSmurfStatePoolIndex, functionType[i]);
      printf(" Top Vble: %d", functions[i]->variable);
      if (functionType[i] > 0)
      {
         printf(" LHS Vble: %d", equalityVble[i]);
      }
      printf("\n");
#endif
   }
   d3_printf1("\n");

   assert(nRegSmurfIndex == nNumRegSmurfs);
   if (nNumRegSmurfs) {
   }

   if (nSpecialFuncIndex) {
      for(int i=0; i < nSpecialFuncIndex; i++)
      {
         if (arrSpecialFuncs[i].nFunctionType == SFN_XOR &&
               arrSpecialFuncs[i].LinkedSmurfs != -1) {
            int smurf = arrSpecialFuncs[i].LinkedSmurfs;
            if (smurf != -1) {
               arrSmurfChain[smurf].specfn = i;
            }
         }
      }
   }

   free(arrIte2SolverSpecFn);
   free(arrIte2SolverSmurf);

   if (nHeuristic == JOHNSON_HEURISTIC)  InitHeuristicTablesForSpecialFuncs();

   // Display statistics.
   double fEndTime = get_runtime();
   ite_counters_f[BUILD_SMURFS] = fEndTime - fStartTime;
   SmurfStatesDisplayInfo();
   d3_printf2 ("Time to build Smurf states:  %4.3f secs.\n", ite_counters_f[BUILD_SMURFS]);

   d4_printf4("SMURF States Statistic: %ld/%ld (%f hit rate)\n",
         (long)(ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]),
         (long)(ite_counters[SMURF_NODE_FIND]),
         ite_counters[SMURF_NODE_FIND]==0?0:1.0 * (ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]) / ite_counters[SMURF_NODE_FIND]);
   d4_printf2 ("Avg. number of vars per smurf  %4.3f \n", 
         1.0*(nSumVarsPerSmurf/(nRegSmurfIndex>0?nRegSmurfIndex:1)));
   d2e_printf1("\rCreating Smurfs ... Done                     \n");

   return SOLV_UNKNOWN;
}


ITE_INLINE void
InitSmurfFactory()
{
   d9_printf1("InitSmurfFactory\n");
   d2e_printf1("Creating Smurfs ... ");
#ifdef DISPLAY_TRACE
   for (int i = 0; i < nmbrFunctions; i++)
   {
      cout << "Constraint " << i << ":" << endl;
      dbPrintBDD(functions[i]);
   }
#endif

   // Initialize info regarding the 'true' function.
   pTrueSmurfState = AllocateSmurfState();

   // Count number of special functions
   // See "struct SpecialFunc" in SmurfFactory.h for a description
   // of what a "special function" is.
   nNumSpecialFuncs = 0;
   nNumRegSmurfs = 0;
   for (int i = 0; i < nmbrFunctions; i++)
   {
      int nFunctionType = functionType[i];

      if (nFunctionType != UNSURE && nFunctionType != ITE && nFunctionType != ITE_EQU && !IsSpecialFunc(nFunctionType))
      {
         d2_printf3("SmurfFactory -- Unrecognized function type: %d for %d assuming UNSURE\n", 
               nFunctionType, i);
         nFunctionType = UNSURE;
      }

      if (xorFunctions[i]) {
         nNumSpecialFuncs++;
         nNumRegSmurfs++;
      } else
      if (IsSpecialFunc(nFunctionType)) 
         nNumSpecialFuncs++;
      else
         nNumRegSmurfs++;

   }

   /* allocate an array for special functions */
   d3_printf2("Number of special functions: %d\n", nNumSpecialFuncs);
   if (nNumSpecialFuncs > 0)
   {
      arrSpecialFuncs
         = (SpecialFunc *)ite_calloc(nNumSpecialFuncs+1, sizeof(SpecialFunc),
               2, "special function array");
   }
   nSpecialFuncIndex = 0;

   /* allocate an array for smurf initial states */
   d3_printf2("Number of regular Smurfs: %d\n", nNumRegSmurfs);
   if (nNumRegSmurfs > 0)
   {
      arrRegSmurfInitialStates
         = (SmurfState **)ite_calloc(nNumRegSmurfs, sizeof(SmurfState *),
               2, "initial smurf state pointers");
      arrSmurfEqualityVble = (int*)ite_calloc(nNumRegSmurfs, sizeof(int),
            9, "arrSmurfEqualityVble");
      arrSmurfPath = (t_smurf_path*)ite_calloc(nNumRegSmurfs, sizeof(t_smurf_path),
            9, "arrSmurfPath");

      arrSmurfChain = (t_smurf_chain*)ite_calloc(nNumRegSmurfs, sizeof(t_smurf_chain),
            9, "arrSmurfChain");

      for (int i = 0; i < nNumRegSmurfs; i++)
      {
         arrSmurfChain[i].next = -1;
         arrSmurfChain[i].prev = -1;
         arrSmurfChain[i].specfn = -1;
      }

   }
   nRegSmurfIndex = 0;

}

void
CountSmurfsUsage(SmurfState *pState, long *one, long *two, long *three)
{
   if (pState->cVisited == 0) return; else
   if (pState->cVisited == 1) (*one)++; else
   if (pState->cVisited == 2) (*two)++; else
   if (pState->cVisited == 3) (*three)++; else assert(0);
   pState->cVisited = 0;

   for(int i=0; i<pState->vbles.nNumElts; i++) {
      Transition *pTransition;
      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
      if (pTransition->pNextState) CountSmurfsUsage(pTransition->pNextState, one, two, three);
      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
      if (pTransition->pNextState) CountSmurfsUsage(pTransition->pNextState, one, two, three);
   }
}

ITE_INLINE void
FreeSmurfFactory()
{
   d9_printf1("FreeSmurfFactory\n");

   long states_one=0, states_two=0, states_three=0;
   for (int i = 0; i < nNumRegSmurfs; i++) {
      CountSmurfsUsage(arrRegSmurfInitialStates[i], &states_one, &states_two, &states_three);
   }

   d3_printf4("SmurfState State all: %lld, visited: %ld, heuristic: %ld\n",
         ite_counters[SMURF_NODE_NEW], states_one+states_three, states_two);

   if (nHeuristic == JOHNSON_HEURISTIC) {
      FreeHeuristicTablesForSpecialFuncs();
   }

   /* from smurf factory */
   for (int i = 0; i < nNumRegSmurfs; i++)
      ite_free((void**)&arrSmurfPath[i].literals);

   ite_free((void**)&arrSmurfPath);
   ite_free((void**)&arrSmurfChain);
   ite_free((void**)&arrSmurfEqualityVble);
   ite_free((void**)&arrRegSmurfInitialStates);
   nRegSmurfIndex = 0;

   for(int i=0; i < nNumSpecialFuncs; i++) {
      ite_free((void**)&arrSpecialFuncs[i].rhsVbles.arrElts);
      ite_free((void**)&arrSpecialFuncs[i].arrRHSPolarities);
      ite_free((void**)&arrSpecialFuncs[i].arrShortLemmas);
   }
   ite_free((void**)&arrSpecialFuncs);
   nSpecialFuncIndex = 0;

   FreeSmurfStatePool();
}

