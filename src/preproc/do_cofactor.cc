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

#include "sbsat.h"
#include "sbsat_preproc.h"

int COF_MAX = 100;		//The max number of variables BDDs can have to be used in cofactoring

int
Do_Cofactor()
{
  int ret=PREP_NO_CHANGE;
  BDDNode *cof = NULL;
  dX_printf(3, "COFACTORING - ");
	affected = 0;
	char p[100];
    sprintf(p, "{0:0/%d}", nmbrFunctions);
    str_length = dX_printf(3, p);
	
  for (int x = 0; x < numout; x++)
   {
    if (x % ((nmbrFunctions/100)+1) == 0) {
        for(int iter = 0; iter<str_length; iter++)
            dX_printf(3,"\b");
        sprintf(p, "{%ld:%d/%d}", affected, x, nmbrFunctions);
        str_length = dX_printf(3, p);
    }

      if (x % ((nmbrFunctions/100)+1) == 0) {
			if (nCtrlC) {
				dX_printf(3, "\nBreaking out of CoFactoring\n");
				nCtrlC = 0;
				break;
			}
         d2e_printf3("\rPreprocessing Co %d/%ld", x, numout);
		}
      
		if (length[x] <= COF_MAX && length[x] > 0) {
			//dX_printf(3, "*"); 
			cof = functions[x];
			for (int j = 0; j < numout; j++) {
				if(j == x) continue;
				if (nmbrVarsInCommon (x, j, 1) == 0) continue;
				BDDNode *before = gcf(functions[j], cof);
				if(before != functions[j]) {
					functions[j] = before;
					if (functions[j] == false_ptr)
					  return TRIV_UNSAT;
					functionType[j] = UNSURE;
					equalityVble[j] = 0;
					SetRepeats(j);
					switch (Rebuild_BDDx(j)) {
					 case TRIV_UNSAT: return TRIV_UNSAT;
					 case TRIV_SAT:   return TRIV_SAT; 
					 default: break;
					}
					affected++;
				}
			}
			functions[x] = true_ptr;
			functionType[x] = UNSURE;
			equalityVble[x] = 0;
			SetRepeats(x);
			switch (Rebuild_BDDx(x)) {
			 case TRIV_UNSAT: return TRIV_UNSAT;
			 case TRIV_SAT:   return TRIV_SAT;
			 default: break;
			}
			affected++;
			ret = PREP_CHANGED;			
		}
		if(ret == PREP_CHANGED) break;
   }  //This ends Cofactoring
	dX_printf(3, "\n");
	d2e_printf1("\r                                                    ");
	return ret;
}
