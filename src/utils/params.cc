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
#include "ite.h"
#include "solver.h"
#include "params.h"

void ctrl_c_proc(int x);

char sHeuristic[10]="j";
char cnfformat[128]="noqm";
int  n_cnfformat=CNF_NOQM;
char sResult[4]="n";
int  ctrl_c=0;
int  nCtrlC=0; /* ctrl c pressed */
long long ite_counters[MAX_COUNTER];
double ite_counters_f[MAX_COUNTER_F];
int  autarky=0; /* autarkies enabled */
int  compress_smurfs=0;
char debug_dev[128]="stderr";

extern long nTimeLimit;
extern long nNumChoicePointLimit;
extern char temp_dir[128];
extern char ini_filename[128];
extern char input_result_filename[128];
extern char preproc_string[256];
extern int  backjumping;
extern int  NO_LEMMAS;

void DO_ALL(int);

t_opt options[] = { 
// p_target, l_opt, w_opt, p_type, p_value, p_defa, var_type, p_src=0, desc_opt

/* 
 * General SBSAT options 
 */
{ (void*)show_help, "h",   "help", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
              "Show all program options"},
{ (void*)show_version, "", "version", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
              "Show program version"},
{ (void*)show_ini, "", "create-ini", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
              "Create ini file"},
{ &ini_filename, "", "ini", P_STRING, 
	       V(i:127,"127"), {"~/ite.ini"}, VAR_CMD+VAR_DUMP, 0,
              "Set the ini file"},
{ &DEBUG_LVL, "",   "debug", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
              "debugging level (0-none, 9-max)"},
{ &debug_dev, "",   "debug-dev", P_STRING, V(i:127,"127"), {"stderr"}, VAR_NORMAL, 0,
              "debugging device"},
{ &PARAMS_DUMP, "D",   "params-dump",   P_PRE_INT,  V(i:1,"1"), V(i:0,"0"), VAR_CMD, 0,
               "dump all internal parameters before processing"},
{ inputfile, "", "input-file", P_STRING, V(i:127,"127"), {"-"}, VAR_NORMAL, 0, 
                "input filename"},
{ outputfile, "", "output-file", P_STRING, V(i:127,"127"), {"-"}, VAR_NORMAL, 0, 
                "output filename"},
{ temp_dir, "", "temp-dir", P_STRING, 
		V(i:127,"127"), {"$TEMP"}, VAR_NORMAL, 0, 
                "directory for temporary files"},
{ sResult,   "R",   "show-result",   P_STRING,  V(i:1,"1"), {"n"}, VAR_NORMAL, 0,
               "Show result (n=no result, r=raw, f=fancy)"},
{ &verify_solution,   "",  "verify-solution",   P_INT,  V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
               "Verify solution"},
{ s_expected_result, "",  "expected-result", P_STRING, V(i:127,"127"), {""}, VAR_NORMAL, 0,
               "Report error if the result is not as specified\nOptions are SAT, UNSAT, TRIV_SAT, TRIV_UNSAT, SOLV_SAT and SOLV_UNSAT"},
/*
{ &input_result_filename, "", "read-result-filename", P_STRING, 
	       V(i:127,"127",}, V(s:,,""), VAR_NORMAL, 0,
              "Filename with the partial assignment"},
*/
{ &ctrl_c,  "",   "ctrl-c",   P_INT,  V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
               "Enable/Disable Ctrl-c handler to end preproc/brancher"},
{ &reports,  "",   "reports",   P_INT,  V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
               "Reporting style during branching (0 - standard, 1 - crtwin)"},

/* 
 * BDD options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nBDD options:"},
{ &numBuckets, "", "num-buckets", P_INT, V(i:0,"0"), V(i:16,"16"), VAR_NORMAL, 0, 
                "Set the number of buckets in power of 2"},
{ &sizeBuckets, "", "size-buckets", P_INT, V(i:0,"0"), V(i:5,"5"), VAR_NORMAL, 0, 
                "Set the size of a bucket in power of 2"},

/* 
 * Input options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nInput options:"},
{ &AND_EQU_LIMIT, "", "limit-and-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function and_equ"},
{ &OR_EQU_LIMIT, "", "limit-or-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function or_equ"},
{ &PLAINOR_LIMIT, "", "limit-or", P_INT, V(i:0,"0"), V(i:8,"8"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function plainor"},
{ &PLAINXOR_LIMIT, "", "limit-xor", P_INT, V(i:0,"0"), V(i:5,"5"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function plainxor"},
{ &BREAK_XORS, "", "break-xors", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0, 
                "Break XORS into linear and non-linear functions"},


/* 
 * Output options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nOutput options:"},
{ &formatout, "b", "", P_PRE_CHAR, V(c:'b',"b"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Start regular brancher"},
{ &formatout, "w", "", P_PRE_CHAR, V(c:'w',"w"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Start walksat brancher"},
{ &formatout, "m", "", P_PRE_CHAR, V(c:'m',"m"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Start WVF brancher"},
{ &formatout, "n", "", P_PRE_CHAR, V(c:'n',"n"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Don't start any brancher or conversion"},
{ &formatout, "s", "", P_PRE_CHAR, V(c:'s',"s"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in SMURF format"},
{ &formatout, "c", "", P_PRE_CHAR, V(c:'c',"c"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in CNF format"},
{ &formatout, "p", "", P_PRE_CHAR, V(c:'p',"p"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in tree like format"},
{ cnfformat,  "", "cnf", P_STRING, V(i:127,"127"), {"noqm"}, VAR_NORMAL, 0, 
                "Format of CNF output (3sat, qm, noqm)"},

/* b s c w   - left ldxpi  */

{ &formatin,  "", "formatin", P_CHAR, V(i:0,"0"), V(c:' '," "), VAR_DUMP, 0, 
                "Input format"},
{ &print_tree, "tree", "tree", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_CMD, 0, 
                "Print tree representation"},
{ &PRINT_TREE_WIDTH, "W", "tree-width", P_INT, V(i:0,"0"), V(i:64,"64"), VAR_NORMAL, 0, 
                "Set tree width"},

/* 
 * Trace format options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nTrace format options:"},
{ module_root, "", "module-dir", P_STRING, 
		V(i:127,"127"), {"./Modules"}, VAR_NORMAL, 0, 
                "directory to find extra modules"},
{ &tracer5, "", "tracer5", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_CHECK, 0, 
                "Use alternative tracer"},

/* 
 * Preprocessing options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nPreprocessing options:"},
{ preproc_string, "P", "preprocess-sequence", P_STRING, 
		V(i:255,"255"), {"(ExDc)*(ExSt)*(ExPr)*"}, VAR_NORMAL, 0, 
                "The preprocessing sequence"},
{ (void*)DO_ALL, "All",  "All", P_FN_INT, V(i:0,"0"), V(i:2,"2"), VAR_CMDLINE+VAR_DUMP, 0, 
	       "Enable/Disable All Preprocessing Options (1/0)"},
/*
{ &DO_ALL, "All:ON",  "All:ON", P_PRE_INT, {i:1}, {i:2}, VAR_NORMAL, 0, 
	       "Enable All Preprocessing Options"},
{ &DO_ALL, "All:OFF", "All:OFF", P_PRE_INT, {i:0}, {i:2}, VAR_NORMAL, 0, 
		"Disable All Preprocessing Options"},
*/
{ &DO_CLUSTER, "Cl",  "Cl", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0, 
	       "Enable/Disable Clustering (1/0)"},
{ &DO_COFACTOR, "Co",  "Co", P_INT, V(i:0,"0"), V(i:0,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Cofactoring (1/0)"},
{ &DO_PRUNING, "Pr",  "Pr",   P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Pruning (1/0)"},
{ &DO_STRENGTH, "St",  "St",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Strengthening (1/0)"},
{ &DO_INFERENCES, "In",  "In", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Inferences (1/0)"},
{ &DO_EXIST_QUANTIFY, "Ex",  "Ex", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Existential Quantification (1/0)"},
{ &DO_EXIST_QUANTIFY_AND, "Ea",  "Ea",  P_INT,  V(i:0,"0"),  V(i:0,"0"), VAR_CHECK+VAR_DUMP, 0,
		"Enable/Disable AND-Existential Quantification (1/0)"},
{ &DO_DEP_CLUSTER, "Dc",  "Dc",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Dependent Variable Clustering (1/0)"},
{ &max_preproc_time, "",  "max-preproc-time", P_INT, V(i:0,"0"),  V(i:0,"0"), VAR_NORMAL, 0,
		"set the time limit in seconds (0=no limit)"},
	
/* 
 * Brancher options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nBrancher options:"},
{ &reverse_independant_dependant, "r", "reverse-depend", P_PRE_INT, 
	     	V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0, 
		"Reverse dependency info on in/dependent variables"},
{ &clear_dependance, "e",  "clear-depend", P_PRE_INT, 
		V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0, 
		"Clear dependency information on variables"},
{ sHeuristic, "H", "heuristic", P_STRING, V(i:8,"8"), {"j"}, VAR_NORMAL, 0,
		"Choose heuristic j=Johnson, l=Chaff-like lemma h.\ni=Interactive"},
{ &backjumping, "", "backjumping", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
		"Enable/Disable backjumping (1/0)"},
{ &MAX_NUM_CACHED_LEMMAS, "L", "max-cached-lemmas", P_INT, 
		V(i:0,"0"), V(i:5000, "5000"), VAR_NORMAL, 0,
                "set the maximum # of lemmas"}, 
{ &autarky, "", "autarky", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_CHECK+VAR_DUMP, 0,
		"Enable/Disable autarkies (1/0)"},
{ &max_solutions, "", "max-solutions", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
		"Set the maximum number of solutions to search for."},
{ &sbj, "", "sbj", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"Super backjumping."},
/*
{ NULL, "", "lemma-space-size", P_LONG, 
		V(i:0,"0"), V(l:LEMMA_SPACE_SIZE,"0"), VAR_DUMP, 0,
                "size of the increments to lemma space"}, 
*/
{ &MAX_VBLES_PER_SMURF, "S", "max-vbles-per-smurf", P_INT, 
		V(i:0,"0"), V(i:8,"8"), VAR_NORMAL, 0,
		"set the maximum number variables per smurf"},
{ &BACKTRACKS_PER_STAT_REPORT, "", "backtracks-per-report", P_INT, 
		V(i:0,"0"), V(i:10000,"10000"), VAR_NORMAL, 0,
		"set the number of backtracks per report"},
{ &nTimeLimit, "", "max-brancher-time", P_INT, 
		V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"set the time limit in seconds (0=no limit)"},
{ &nNumChoicePointLimit, "", "max-brancher-cp", P_INT, 
		V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"set the choice point limit (0=no limit)"},
{ &TRACE_START, "", "brancher-trace-start", P_INT, 
		V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"number of backtracks to start the trace (when debug=9)"},
{ &compress_smurfs, "", "compress-smurfs", P_INT, 
		V(i:1,"1"), V(i:1,"1"), VAR_NORMAL, 0,
		"Share states among smurfs"},
{ &SMURFS_SHARE_PATHS, "", "smurfs-share-paths", P_INT, 
		V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
		"Share paths among smurfs"},
/*
{ png_filename, "", "png-filename", P_STRING, 
		V(i:128,"128"), "", VAR_CHECK+VAR_DUMP, 0,
		"Png filename for brancher graphs"},
*/
{ jpg_filename, "", "jpg-filename", P_STRING, 
		V(i:127,"127"), {""}, VAR_CHECK+VAR_DUMP, 0,
		"Jpg filename for brancher graphs"},

/* 
 * Johnson heuristic options 
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nJohnson heuristic options:"},
{ &JHEURISTIC_K, "K", "jheuristic-k", P_FLOAT, 
		V(i:0,"0"), V(f:3.0, "3.0"), VAR_NORMAL, 0,
		"set the value of K"},
{ &JHEURISTIC_K_TRUE, "", "jheuristic-k-true", P_FLOAT, 
		V(i:0,"0"), V(f:0.0, "0.0"), VAR_NORMAL, 0,
		"set the value of True state"},
{ &JHEURISTIC_K_INF, "", "jheuristic-k-inf", P_FLOAT, 
		V(i:0,"0"), V(f:1.0, "1.0"), VAR_NORMAL, 0,
		"set the value of the inference multiplier"},
/* 
 * the end
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, 0, 0, ""}
};

/*
"MAX_NUM_PRIME_IMPLICATS: %ld\n",(long)MAX_NUM_PRIME_IMPLICANTS);
"MAX_NUM_PATH_EQUIV_SETS: %ld\n",(long)MAX_NUM_PATH_EQUIV_SETS);
"MAX_EXQUANTIFY_CLAUSES: %d\n", MAX_EXQUANTIFY_CLAUSES);
"COF_MAX: %d\n",                COF_MAX);
"MAX_EXQUANTIFY_VARLENGTH: %d\n", MAX_EXQUANTIFY_VARLENGTH);
"MAXFILE_LENGTH: %ld\n",     (long)MAXFILE_LENGTH);
"LINE_LENGTH: %ld\n",        (long)LINE_LENGTH);
"SIZE_SMURF_STATE_POOL: %ld\n", (long)SIZE_SMURF_STATE_POOL);
*/
// "HEURISTIC: %d\n",               nHeuristic);
// "STATE_TIME: %f\n",        STATE_TIME);

// 
// l_opt,  w_opt, function
// { "c",  "create_ini_file",  create_ini_file }
// { "h",  "help",             show_help }
//

void
finish_params()
{
   if (!strcmp(debug_dev, "stdout")) stddbg = stdout;
   else
   if (!strcmp(debug_dev, "stderr")) stddbg = stderr;
   else {
   }


   t_opt *p_opt;
   char a_opt[][255] = { "temp-dir", /*"-any-other-par-",*/ "" };


   for (int i=0; a_opt[i][0]!=0; i++)
   { 
      p_opt = lookup_keyword(a_opt[i]);
      if (!p_opt) continue;
      if (((char*)(p_opt->p_target))[0] == '~') {
         char *env = getenv("HOME");
         if (env) 
         {
            char temp_str[256];
            sprintf(temp_str, "%s%s", 
                  env, ((char*)(p_opt->p_target))+1);
            strncpy((char*)(p_opt->p_target), temp_str,
                  p_opt->p_value.i-1);
            ((char*)(p_opt->p_target))[p_opt->p_value.i]=0;
         }
      }
      if (!strncmp((char*)(p_opt->p_target), "$TEMP", 5)) {
         char *env = getenv("TEMP");
         if (!env) env = getenv("TMPDIR");
         if (!env) {
            struct stat buf;
            if (stat("/tmp", &buf) == 0) env = "/tmp";
         }
         if (env)
         {
            char temp_str[256];
            sprintf(temp_str, "%s%s", 
                  env, ((char*)(p_opt->p_target))+5);
            strncpy((char*)(p_opt->p_target), temp_str,
                  p_opt->p_value.i-1);
            ((char*)(p_opt->p_target))[p_opt->p_value.i]=0;
         } else {
            d1_printf1 ("TEMP or TMPDIR variable not found.\n");
         }
      }
   }

   switch (sResult[0]) {
    case 'n': result_display_type = 0; break;
    case 'r': result_display_type = 1; break;
    case 'f': result_display_type = 2; break;
    default: 
              dE_printf2("error: Unknown result type %c\n", sResult[0]); 
              exit(1);
              break;
   }

   switch (sHeuristic[0]) {
    case 'j': nHeuristic = JOHNSON_HEURISTIC; break;
    case 'l': nHeuristic = C_LEMMA_HEURISTIC; break;
    case 'i': nHeuristic = INTERACTIVE_HEURISTIC; break;
    case 'm': nHeuristic = STATE_HEURISTIC; break;
    default: 
              dE_printf2("error: Unknown heuristic type %c\n", sHeuristic[0]); 
              exit(1);
              break;
   }

   if (MAX_VBLES_PER_SMURF > MAX_MAX_VBLES_PER_SMURF)
   {
      dE_printf2("error: limit for MAX_VBLES_PER_SMURF -S is %d\n",
            MAX_MAX_VBLES_PER_SMURF);
      exit(1);
   }

   MAX_NUM_PATH_EQUIV_SETS=1;
   for (int i=0;i<MAX_VBLES_PER_SMURF;i++)
      MAX_NUM_PATH_EQUIV_SETS*=3;

   if (backjumping == 0) {
      if (MAX_NUM_CACHED_LEMMAS == 0) NO_LEMMAS=1;
      else 
      {
         dE_printf1("error: Can not disable backjumping and have lemmas\n"); 
         dE_printf1("       Please set -L 0\n"); 
         exit(1);
      }
   }

   if (!strcmp(cnfformat, "noqm")) n_cnfformat = CNF_NOQM; else
      if (!strcmp(cnfformat, "qm"))   n_cnfformat = CNF_QM; else
         if (!strcmp(cnfformat, "3sat")) n_cnfformat = CNF_3SAT; else
         {
            dE_printf2("error: Unrecognized CNF Format: %s\n", cnfformat); 
            exit(1);
         }

   if (ctrl_c == 1) {
      signal (SIGINT, ctrl_c_proc);
   }
}

int
params_parse_cmd_line(int argc, char *argv[])
{
   init_options();
   read_cmd(argc, argv);
   fix_ini_filename();
   read_ini(ini_filename);
   finish_params();

   if (PARAMS_DUMP) dump_params();
   return 0;
}

void 
ctrl_c_proc(int x)
{
   fprintf(stderr, "Please hold on for phase interruption\n");
   nCtrlC = 1;
}

void
DO_ALL(int value)
{
   DO_CLUSTER = value;
   DO_COFACTOR = value;
   DO_PRUNING = value;
   DO_STRENGTH = value;
   DO_EXIST_QUANTIFY = value;
   DO_EXIST_QUANTIFY_AND = value;
   DO_DEP_CLUSTER = value;
}
