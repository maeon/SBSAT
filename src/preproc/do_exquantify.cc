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
 *  preprocess.c (S. Weaver)
 *********************************************************/

#include "ite.h"

//!These two flags should not be changed until ExQuantify is redone!

int MAX_EXQUANTIFY_CLAUSES = 3;	//Number of BDDs a variable appears in
				     //to quantify that variable away.
int MAX_EXQUANTIFY_VARLENGTH = 18;	//Limits size of number of vars in 
				     //constraints created by ExQuantify
//!
int ExQuantify();

int Do_ExQuantify() {
   d2_printf1 ("EXISTENTIALLY QUANTIFYING - ");
   int cofs = PREP_CHANGED;
   int ret = PREP_NO_CHANGE;
   while (cofs!=PREP_NO_CHANGE) {
      cofs = ExQuantify ();
      if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
      else if(cofs == TRIV_UNSAT) {
			return TRIV_UNSAT;
		}
	}
	
	d2_printf1 ("\n");
	return ret;
}

int ExQuantify () {
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
		if(tempinters[i] == 1) {
			int j = tempmem[i]->next->BDD;
			//Only quantify away variables from unknown functions, or functions
			//who have the quantified variable as the 'head' or LHS variable of 
			//their function...a LHS variable is a 'Left Hand Side' variable.
			if ((functionType[j] == OR && length[j] < OR_EQU_LIMIT)
				 || (functionType[j] == AND && length[j] < AND_EQU_LIMIT)
				 || (functionType[j] == PLAINOR)
				 || (i == abs (equalityVble[j]))) {

				Quantify = functions[j];
				d2_printf2 ("*{%d}", i);
				//fprintf(stderr, "\n%d: ", j);
				//printBDDerr(functions[j]);
				//fprintf(stderr, "\n");
				functions[j] = xquantify (Quantify, i);
				variablelist[i].true_false = 2;
				SetRepeats(j);
				equalityVble[j] = 0;
				functionType[j] = UNSURE;
				//fprintf(stderr, "\n");
				//printBDDerr(functions[j]);
				//fprintf(stderr, "\n");
				ret = PREP_CHANGED;
				if (functions[j] == false_ptr) {
					ret = TRIV_UNSAT;
					goto ex_bailout; /* as much as I hate gotos */
				}

				int *temparr = new int[length[j]];
				for(int a = 0; a < length[j]; a++)
				  temparr[a] = variables[j].num[a];
				int oldlength = length[j];
				
				switch (int r=Rebuild_BDDx(j)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: 
					ret=r;
					goto ex_bailout;
					break;
				 default: break;
				}
				

				int *num = variables[j].num;
				if(length[j] == 0) {
					num[0] = numinp+2;
				}
				  
			   int index = 0;
				int done = 0;
				int new_i = 0;
				for(int a = 0; a < oldlength; a++) {
					if(done || temparr[a] < num[index]) {
						//variable 'temparr[a]' got knocked out
						for(llBDD *lltemp = tempmem[temparr[a]]->next; lltemp != NULL; lltemp = lltemp->next) {
							if(lltemp->BDD == j) {
								llBDD *temp = lltemp;
								if(lltemp->next!=NULL) lltemp->next->previous = lltemp->previous;
								lltemp->previous->next = lltemp->next;
								lltemp = lltemp->previous;
								tempinters[temparr[a]]--;
								delete temp;
								break;
							}
						}
					} else {
						index++;
					}
					if(new_i==0 && length[j]!=0) new_i = num[index];
					if(index >= length[j]){
					  done = 1;
					  index = 0;
					}
				}
				
				delete [] temparr;
				if(new_i!=0) i = new_i-1;
				//i = 0;
				//'i' should equal the lowest indexed variable-1 that stayed in.
			}
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
