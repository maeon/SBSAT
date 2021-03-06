#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

// OR State

void initORStateType() {
	arrStatesTypeSize[FN_OR] = sizeof(ORStateEntry);
   arrSetVisitedState[FN_OR] = SetVisitedORState;
	arrApplyInferenceToState[FN_OR] = ApplyInferenceToOR;
	arrPrintStateEntry[FN_OR] = PrintORStateEntry;
	arrPrintStateEntry_dot[FN_OR] = PrintORStateEntry_dot;
	arrFreeStateEntry[FN_OR] = FreeORStateEntry;
	arrCalculateStateHeuristic[FN_OR] = CalculateORLSGBHeuristic;
	arrSetStateHeuristicScore[FN_OR] = LSGBORStateSetHeurScore;
   arrGetStateHeuristicScore[FN_OR] = LSGBORStateGetHeurScore;
}

void PrintORStateEntry(void *pState) {
	ORStateEntry *pORState = (ORStateEntry *)pState;
	d9_printf1("OR Vars=");
	for(int x = 0; x < pORState->nSize; x++)
	  d9_printf2("%d ", pORState->bPolarity[x]==1?pORState->pnTransitionVars[x]:-pORState->pnTransitionVars[x]);
	d9_printf2("Size=%d\n", pORState->nSize);
}

void PrintORStateEntry_dot(void *pState) {
	ORStateEntry *pORState = (ORStateEntry *)pState;
	fprintf(stdout, " b%p->b%p [style=solid,label=\" x1 \"]\n", (void *)pORState, (void *)pTrueSimpleSmurfState);
	fprintf(stdout, " b%p->b%p [style=dotted,label=\" -x1 : x2 \"]\n", (void *)pORState, (void *)pTrueSimpleSmurfState);
	fprintf(stdout, " b%p->b%p [style=solid,label=\" x2 \"]\n", (void *)pORState, (void *)pTrueSimpleSmurfState);
	fprintf(stdout, " b%p->b%p [style=dotted,label=\" -x2 : x1 \"]\n", (void *)pORState, (void *)pTrueSimpleSmurfState);
	
	fprintf(stdout, " b%p [shape=\"ellipse\",label=\"OR, n=%d\"]\n", (void *)pORState, 2);
}

void FreeORStateEntry(void *pState) {
	ORStateEntry *pORState = (ORStateEntry *)pState;
	ite_free((void **)&pORState->bPolarity);
	ite_free((void **)&pORState->pnTransitionVars);
   if(pORState->pORStateBDD != NULL) {
      pORState->pORStateBDD->pState = NULL;
   }
}

// OR Counter State

void initORCounterStateType() {
	arrStatesTypeSize[FN_OR_COUNTER] = sizeof(ORCounterStateEntry);
   arrSetVisitedState[FN_OR_COUNTER] = SetVisitedORCounterState;
	arrApplyInferenceToState[FN_OR_COUNTER] = ApplyInferenceToORCounter;
	arrPrintStateEntry[FN_OR_COUNTER] = PrintORCounterStateEntry;
	arrPrintStateEntry_dot[FN_OR_COUNTER] = PrintORCounterStateEntry_dot;
	arrFreeStateEntry[FN_OR_COUNTER] = FreeORCounterStateEntry;
	arrCalculateStateHeuristic[FN_OR_COUNTER] = CalculateORCounterLSGBHeuristic;
	arrSetStateHeuristicScore[FN_OR_COUNTER] = LSGBORCounterStateSetHeurScore;
   arrGetStateHeuristicScore[FN_OR_COUNTER] = LSGBORCounterStateGetHeurScore;
}

void PrintORCounterStateEntry_dot(void *pState) {
	ORCounterStateEntry *pORCounterState = (ORCounterStateEntry *)pState;
	fprintf(stdout, " b%p->b%p [style=solid,label=\" x\"]\n", (void *)pORCounterState, pTrueSimpleSmurfState);
	fprintf(stdout, " b%p->b%p [style=dotted,label=\" -x\"]\n", (void *)pORCounterState, (void *)(pORCounterState->pTransition));
	
	fprintf(stdout, " b%p [shape=\"ellipse\",label=\"OR, n=%d\"]\n", (void *)pORCounterState, pORCounterState->nSize);
	
	arrPrintStateEntry_dot[(int)((TypeStateEntry *)(pORCounterState->pTransition))->cType](pORCounterState->pTransition);
}

void PrintORCounterStateEntry(void *pState) {
	ORCounterStateEntry *pORCounterState = (ORCounterStateEntry *)pState;
	d9_printf4("OR Next=%p Head=%p Size=%d\n",
				  (void *)pORCounterState->pTransition,
				  (void *)pORCounterState->pORState,
				  pORCounterState->nSize);
}

void FreeORCounterStateEntry(void *pState){
	
}
