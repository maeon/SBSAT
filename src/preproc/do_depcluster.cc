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
/*********************************************************
 *  depcluster.cc (S. Weaver)
 *********************************************************/

#include "ite.h"

int DepCluster();

int Do_DepCluster() {
	d2_printf1 ("DEPENDENT CLUSTERING - ");
   int cofs = PREP_CHANGED;
   int ret = PREP_NO_CHANGE;
	while (cofs!=PREP_NO_CHANGE) {
      cofs = DepCluster ();
		//cofs = Do_ExQuantify ();
      if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
      else if(cofs == TRIV_UNSAT) {
			return TRIV_UNSAT;
		}
	}
	
	d2_printf1 ("\n");
	return ret;
}

int DepCluster () {
	int ret = PREP_NO_CHANGE;
	
	typedef struct LinkedListofBDDs {
	   int BDD;
		LinkedListofBDDs *next;
		LinkedListofBDDs *previous;
	} llBDD;

	int *tempinters = new int[numinp+1];
	llBDD **tempmem = new llBDD*[numinp+1];
	for(int x = 0; x < numinp+1; x++) {
		tempinters[x] = 0;
		tempmem[x] = new llBDD;
		tempmem[x]->BDD = -1;
		tempmem[x]->next = NULL;
		tempmem[x]->previous = NULL;
	}

	for(int x = 0; x < nmbrFunctions; x++) {
		for(int i = 0; i < length[x]; i++) {
			tempinters[variables[x].num[i]]++;
			llBDD *newnode = new llBDD;
			newnode->BDD = x;
			if(tempmem[variables[x].num[i]]->next!=NULL)
			  tempmem[variables[x].num[i]]->next->previous = newnode;
			newnode->next = tempmem[variables[x].num[i]]->next;
			newnode->previous = tempmem[variables[x].num[i]];
			tempmem[variables[x].num[i]]->next = newnode;
		}
	}
	
	BDDNode *Quantify;
	
	for (int i = 0; i < numinp + 1; i++) {
		//do temp variables first (independantVars[i] == 2)
		//then come back and do dependent vars (independantVars[i] == 0)
		if(independantVars[i] != 1) {
			int j = -1;
			int print = 0;
			int count1 = 0;
			int count2 = 0;
			for(llBDD *iter = tempmem[i]->next; iter!=NULL; iter = iter->next) {
				count1++;
				if(Dep_repeat[iter->BDD] != 0) print = 1;
				if(abs(equalityVble[iter->BDD]) == i) {
					if(j!=-1) {
						//fprintf(stderr, "\nInconsistent var %d, skipping\n", i);
						j = -1;
						break;
					}
					j = iter->BDD;
				   break; //comment to ensure dependant vars only show up
					       //on the equal side of one function.
					       //this is not always wanted. XOR recombination
					       //for instance...but it's a good check for trace_dlx
				}
			}
			if(j==-1) continue;
//			if(print == 1) d2_printf2 ("*(%d)", i);
			int depj = Dep_repeat[j];
			Dep_repeat[j] = 0;
			for(llBDD *iter = tempmem[i]->next; iter!=NULL; iter = iter->next) {
				int k = iter->BDD;
				if(Dep_repeat[k] == 0 && depj == 0) continue;
				Dep_repeat[k] = 0;
				if(k == j) continue; //Skip over the function we are using to combine
				//if(length[j] > MAX_VBLES_PER_SMURF) continue;
				//if(length[k] > MAX_VBLES_PER_SMURF) continue;
				int bdd_length = 0;
				int *bdd_vars = NULL;
				
				Quantify = ite_and(functions[j], functions[k]);
				switch (int r=Rebuild_BDD(Quantify, &bdd_length, bdd_vars)) {
				 case TRIV_UNSAT:
				 case PREP_ERROR:
					ret=r;
					goto ex_bailout;
					break;
				 default: break;
				}

				Quantify = xquantify (Quantify, i);
				switch (int r=Rebuild_BDD(Quantify, &bdd_length, bdd_vars)) {
				 case TRIV_UNSAT:
				 case PREP_ERROR:
					ret=r;
					goto ex_bailout;
					break;
				 default: break;
				}

				//if(Quantify == functions[k]) continue;
				
				//d2_printf5 ("*{Ex = %d: (%d & %d) len=%d}", i, j, k, bdd_length);
					
				//fprintf(stderr, "\n");
				//printBDDerr(Quantify);
				//fprintf(stderr, "\n");
					  
				if((functionType[j] == OR || functionType[j] == AND) &&
					(functionType[k] == OR || functionType[k] == AND)) {
					int equ_var = abs(equalityVble[k]);
					BDDNode *true_side = set_variable (Quantify, i, 1);
					BDDNode *false_side = set_variable (Quantify, i, 0);
					if(true_side == ite_not(false_side)) {
						equalityVble[k] = equ_var; //could of been negative, now it's positive for sure
						if(true_side->thenCase == true_ptr || true_side->elseCase == true_ptr)
						  functionType[k] = OR;
						else
						  functionType[k] = AND;
					} else {
						if(bdd_length > MAX_VBLES_PER_SMURF) {	
							if(DO_INFERENCES) {
								switch (int r=Do_Apply_Inferences()) {
								 case TRIV_UNSAT:
								 case PREP_ERROR:
									ret=r;
									goto ex_bailout;
									break;
								 default: break;
								}
							}
							continue;
						}
						continue; //Comment out for REAL dependent clustering
						equalityVble[k] = 0;
						functionType[k] = UNSURE;
					}
				} else {
					if(bdd_length > MAX_VBLES_PER_SMURF) { 
						if(DO_INFERENCES) {
							switch (int r=Do_Apply_Inferences()) {
							 case TRIV_UNSAT:
							 case PREP_ERROR:
								ret=r;
								goto ex_bailout;
								break;
							 default: break;
							}
						}
						continue;
					}
					continue; //Comment out for REAL dependent clustering
					equalityVble[k] = 0;
					functionType[k] = UNSURE;
				}
				
				count2++;
				
				//Need to add BDD k into the inference list of any variable that wasn't
				//originally in k.
				int a = 0, b = 0;
				while(a < bdd_length && b < length[k]) {
					if(bdd_vars[a] > variables[k].num[b]) {
						b++;
					} else if(bdd_vars[a] < variables[k].num[b]) {
						llist *newllist = new llist;
						newllist->num = k;
						newllist->next = NULL;
						amount[bdd_vars[a]].tail->next = newllist;
						amount[bdd_vars[a]].tail = newllist;
						a++;
					} else { a++; b++; }
				}
				
				while(a < bdd_length) {
					llist *newllist = new llist;
					newllist->num = k;
					newllist->next = NULL;
					amount[bdd_vars[a]].tail->next = newllist;
					amount[bdd_vars[a]].tail = newllist;
					a++;
				}
				
				functions[k] = Quantify;
				switch (int r=Rebuild_BDDx(k)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: 
					ret=r;
					goto ex_bailout;
					break;
				 default: break;
				}
				
				SetRepeats(k);
				ret = PREP_CHANGED;
				delete [] bdd_vars;
				bdd_vars = NULL;
			}
			//Dep_repeat[j] = 0;
			if(count1 == (count2+1)) {
				//fprintf(stderr, "HEY!!! %d %d %d Ex this var %d!!!!", count1, count2, tempinters[i], i);
				//Do_ExQuantify();
			}
			if(ret == PREP_CHANGED) goto ex_bailout;
			//lazy...lazy...lazy...
			//could be much faster if i modified tempmem[] to be what it's supposed to be!
		}
	}
		
	ex_bailout:
	for(int x = 0; x < numinp+1; x++) {
		for(llBDD *lltemp = tempmem[x]; lltemp != NULL;) {
			llBDD *temp = lltemp;
			lltemp = lltemp->next;
			delete temp;
		}
	}
	
	delete [] tempmem;
	delete [] tempinters;
	return ret;
}
