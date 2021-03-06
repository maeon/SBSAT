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

LemmaInfoStruct *pFreeLemmaInfoStructPool = NULL;
	
typedef struct _tarrLemmaInfo {
   LemmaInfoStruct *memory;
   struct _tarrLemmaInfo *next;
} tarrLemmaInfo;
tarrLemmaInfo *head_arrLemmaInfo = NULL;


ITE_INLINE void
InitLemmaInfoArray()
{
   LemmaInfoStruct *garrLemmaInfo = NULL;

   assert(MAX_NUM_LEMMAS);
   //assert(garrLemmaInfo==NULL);
   assert(pFreeLemmaInfoStructPool==NULL);
   garrLemmaInfo = (LemmaInfoStruct*)ite_calloc(MAX_NUM_LEMMAS, sizeof(LemmaInfoStruct),
         2, "lemma info array");

   for (int i = 0; i < MAX_NUM_LEMMAS - 1; i++)
   {
      garrLemmaInfo[i].pNextLemma[0] = garrLemmaInfo + i + 1;
   }
   //garrLemmaInfo[MAX_NUM_LEMMAS - 1].pNextLemma[0] = NULL; -- it is NULL already
   pFreeLemmaInfoStructPool = garrLemmaInfo;

   tarrLemmaInfo *t = (tarrLemmaInfo*)ite_calloc(1, sizeof(tarrLemmaInfo), 9, "lemma info array ptr");
   t->memory = garrLemmaInfo;
   if (head_arrLemmaInfo == NULL) { head_arrLemmaInfo = t; t->next = NULL; }
   else t->next = head_arrLemmaInfo;
}

ITE_INLINE void
FreeLemmaInfoArray()
{
   while (head_arrLemmaInfo) {
      tarrLemmaInfo *t = head_arrLemmaInfo->next;
      ite_free((void**)&head_arrLemmaInfo->memory);
      ite_free((void**)&head_arrLemmaInfo);
      head_arrLemmaInfo = t;
   }
   pFreeLemmaInfoStructPool = NULL;
}

ITE_INLINE void
FreeLemmaInfoStruct(LemmaInfoStruct *pLemmaInfo)
   // The LemmaInfoStruct is returned to the pool pointed to by
   // pFreeLemmaInfoStructPool.
{
   // Put it in the global pool.
   pLemmaInfo->pNextLemma[0] = pFreeLemmaInfoStructPool;
   pLemmaInfo->pNextLemma[1] = NULL;
   pLemmaInfo->pPrevLemma[0] = NULL;
   pLemmaInfo->pPrevLemma[1] = NULL;
   pFreeLemmaInfoStructPool = pLemmaInfo;
}


ITE_INLINE LemmaInfoStruct *
AllocateLemmaInfoStruct()
{
   if (!pFreeLemmaInfoStructPool)
   {
      // No available LemmaInfoStructs.
      // allocate more LemmaInfoStructs
      d9_printf1("no pFreeLemmaInfoStructPool available\n");
      InitLemmaInfoArray();
   }

   assert(pFreeLemmaInfoStructPool);

   LemmaInfoStruct *pReturn = pFreeLemmaInfoStructPool;
   pFreeLemmaInfoStructPool = pFreeLemmaInfoStructPool->pNextLemma[0];
   return pReturn;
}
