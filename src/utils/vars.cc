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


#include <ite.h>

//float STATE_TIME = 5000.0;
int PRINT_TREE_WIDTH = 64;
int reverse_independant_dependant = 0;
int clear_dependance = 0;
int backjumping = 1;
char formatin = 'f', formatout = 'b'; //Solving the problem is the Default
int print_tree = 0;
char inputfile[128] = "-";
char outputfile[128] = "-";
char progname[65] = "ite";
FILE * finputfile = NULL;
FILE * foutputfile = NULL;
char temp_dir[128] = "";
char ini_filename[128] = "";
int  result_display_type = 0;
char module_root[128] = "./Modules";
char input_result_filename[128] = "";

int DO_CLUSTER = 1;   //CNF clustering
int DO_COFACTOR = 1;  //Cofactoring does no good for any benchmark i've tried
                       //Except small xor-chain files.
int DO_PRUNING = 1;
int DO_STRENGTH = 1;
int DO_INFERENCES = 1;
int DO_EXIST_QUANTIFY = 1;
int DO_EXIST_QUANTIFY_AND = 1;
int DO_DEP_CLUSTER = 1;

int NO_LEMMAS = 0;

int PARAMS_DUMP = 0;

int MAX_NUM_CACHED_LEMMAS = _MAX_NUM_CACHED_LEMMAS;
int BACKTRACKS_PER_STAT_REPORT = _BACKTRACKS_PER_STAT_REPORT;
int nHeuristic = JOHNSON_HEURISTIC;
int MAX_VBLES_PER_SMURF=_MAX_VBLES_PER_SMURF;
long MAX_NUM_PATH_EQUIV_SETS; /* 3^MAX_VBLES_PER_SMURF */
float JHEURISTIC_K=_JHEURISTIC_K;
float JHEURISTIC_K_TRUE=0;
float JHEURISTIC_K_INF=1;

long nTimeLimit = 0;
long nNumChoicePointLimit = 0;



/* former CircuitStruct */
int nmbrFunctions=0;
struct BDDNodeStruct **functions=NULL;
int *functionType=NULL;
int *equalityVble=NULL; ///< Variable on the LHS of an ite=, and=, or or= BDD.
int *independantVars=NULL;
char **labels=NULL;
int **parameterizedVars=NULL;
int *parameterGroup=NULL;

int numBuckets=0;
int sizeBuckets=0;
char preproc_string[256]="((ExSt)Ex(PrSt))";

int TRACE_START=0;

//char png_filename[128]="";
char jpg_filename[128]="";

int ZERO_ONE_VAR=((1 << 20) - 1);

int AND_EQU_LIMIT=0;
int OR_EQU_LIMIT=0;
int PLAINXOR_LIMIT=0;
int PLAINOR_LIMIT=0;

int verify_solution=1;
int tracer5=0;

char s_expected_result[128]="";

int  max_solutions=0;

int BREAK_XORS=0;
int SMURFS_SHARE_PATHS=1;

int max_preproc_time=0;

int sbj = 0;

int reports = 0;
