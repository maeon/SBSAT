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

int COF_MAX = 100;		//The max number of variables BDDs can have to be used in cofactoring

int
Do_Cofactor()
{
  int ret=PREP_NO_CHANGE;
  BDDNode *cof = NULL;

  for (int x = 0; x < numout; x++)
   {
     if (length[x] <= COF_MAX && length[x] > 0)
      {
        d2_printf1 ("*");
        cof = functions[x];
        for (int j = 0; j < numout; j++)
         {
           //Cannot discriminate...must do EVERY function
           BDDNode *before = gcf(functions[j], cof);
			  if(before != functions[j]) {
				  functions[j] = gcf (functions[j], cof);
				  if (functions[j] == false_ptr)
					 return TRIV_UNSAT;
				  functionType[j] = UNSURE;
				  equalityVble[j] = 0;
				  switch (Rebuild_BDDx(j)) {
					case TRIV_UNSAT: return TRIV_UNSAT; break;
					case TRIV_SAT:   return TRIV_SAT; break;
					default: break;
				  }
				  ret = PREP_CHANGED;
			  }
         }
      }
   }  //This ends Cofactoring
  return ret;
}
