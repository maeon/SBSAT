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
#ifndef PREPROC_H
#define PREPROC_H

/* preprocessor result codes */
#define TRIV_UNSAT      0
#define TRIV_SAT        1
#define PREP_CHANGED    2
#define PREP_NO_CHANGE  3
#define PREP_ERROR      4
#define PREP_MAX        5

/* solver result codes */
#define SOLV_SAT	6
#define SOLV_UNSAT	7
#define SOLV_UNKNOWN	8
#define SOLV_ERROR	9

/* conversion result codes */
#define CONV_OUTPUT    10	

extern Linear *l;
extern int *length;
extern int *tempint;
extern store *variables;
extern infer *lastinfer;
extern int notdone;
extern infer *inferlist;
extern varinfo *variablelist;
extern llistStruct *amount;
extern int *P1_repeat;
extern int *P2_repeat;
extern int *Restct_repeat;
extern int *ReFPS_repeat;
extern int *St_repeat;
extern int *Dep_repeat;

void Init_Repeats();
void Delete_Repeats();
void SetRepeats(int x);
void UnSetRepeats(int x);

int Rebuild_BDDx (int x);
int Rebuild_BDD (BDDNode *, int *, int *&);
int Do_Apply_Inferences ();
int Do_Apply_Inferences_backend ();
int Do_Strength();
int Do_Pruning();
int Do_Pruning_1();
int Do_Pruning_2();
int Do_Pruning_Restrict();
int Do_Cofactor();
int Do_ExQuantify();
int Do_ExQuantifyAnd();
int Do_DepCluster();

int Finish_Preprocessing();
int Init_Preprocessing();

int CreateInferences();

extern int signs_idx;
extern char signs[];

#endif
