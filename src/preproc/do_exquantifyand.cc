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

extern int MAX_EXQUANTIFY_CLAUSES; /* = 20;*/	//Number of BDDs a variable appears in
				     //to quantify that variable away.
extern int MAX_EXQUANTIFY_VARLENGTH; /* = 18; */	//Limits size of number of vars in 
				     //constraints created by ExQuantify
//!

int ExQuantifyAnd();

int
Do_ExQuantifyAnd()
{
	MAX_EXQUANTIFY_CLAUSES = 40;
	MAX_EXQUANTIFY_VARLENGTH = 13;
	d2_printf1 ("ANDING AND EXISTENTIALLY QUANTIFYING - ");
	int cofs = PREP_CHANGED;
	int ret = PREP_NO_CHANGE;
	while (cofs!=PREP_NO_CHANGE)
	  {
		  cofs = ExQuantifyAnd ();
		  if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
		  else if(cofs == TRIV_UNSAT)
			 {
				 return TRIV_UNSAT;
			 }
	  }
	d2_printf1 ("\n");
	return ret;
}


int 
ExQuantifyAnd ()
{
  int ret = PREP_NO_CHANGE;

  store *examount = new store[numinp + 1];
  int *tempmem = new int[numinp + 1];
  BDDNode *Quantify;
  //int exquant = 0;   
  for (int x = 0; x < numinp + 1; x++)
    tempmem[x] = 0;
	
  for (int x = 0; x < nmbrFunctions; x++)
	  {
		  for (int i = 0; i < length[x]; i++)
			 {
				 tempmem[variables[x].num[i]]++;
			 }
	  }
	
	for (int x = 0; x < numinp + 1; x++)
	  {
		  examount[x].num = new int[tempmem[x] + 1];
		  examount[x].length = 0;
	  }
	delete tempmem;
	
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  for (int i = 0; i < length[x]; i++)
			 {
				 examount[variables[x].num[i]].num[examount[variables[x].num[i]].
															  length] = x;
				 examount[variables[x].num[i]].length++;
			 }
	  }
	for (int x = 1; x <= MAX_EXQUANTIFY_CLAUSES; x++)
	  {
		  for (int i = 1; i < numinp + 1; i++)
			 {
//				 fprintf(stderr, "%d\n", i);
				 if ((examount[i].length <= x) && (examount[i].length > 0))
					{
						int j = examount[i].num[0];
						Quantify = functions[j];
						int out = 0;
						if(x==1) ret = PREP_CHANGED;
						if(length[j]>MAX_EXQUANTIFY_VARLENGTH) continue;
						for(int z = 1; z < examount[i].length; z++) {
							if(length[examount[i].num[z]] > MAX_EXQUANTIFY_VARLENGTH){
								out = 1;
								break;
							}
							Quantify = ite_and(Quantify, functions[examount[i].num[z]]);
							functions[examount[i].num[z]] = true_ptr;
							UnSetRepeats(examount[i].num[z]);
							equalityVble[examount[i].num[z]] = 0;
							functionType[examount[i].num[z]] = UNSURE;
							length[examount[i].num[z]] = 0;
							if(variables[examount[i].num[z]].num!=NULL)
							  delete variables[examount[i].num[z]].num;
							variables[examount[i].num[z]].num = NULL;
							ret = PREP_CHANGED;
						}
						if(ret != PREP_CHANGED) continue;
						if(out) {
							functions[j] = Quantify;
						} else {
							d2_printf2 ("*{%d}", i);
							functions[j] = xquantify (Quantify, i);
						}
						switch (int r=Rebuild_BDDx(j)) {
						case TRIV_UNSAT: 
						case TRIV_SAT: 
						case PREP_ERROR: 
						   ret = r; goto ea_bailout; /* as much as I hate gotos */
                                                   break;
						default: break;
						}
						equalityVble[j] = 0;
						functionType[j] = UNSURE;
						ret = PREP_CHANGED;
						goto ea_bailout; /* as much as I hate gotos */
					}
			 }
	  }
ea_bailout:
	for (int z = 0; z < numinp + 1; z++)
	  delete examount[z].num;
	delete examount;
	return ret;
}
