/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */ 
/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:
 * 
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
 * 
 * 
 --- This software and any associated documentation is provided "as is"
 * 
 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.
 * 
 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
 *********************************************************************/
/*
 *
 * SBSAT - University of Cincinnati
 *
 */
/**********************************************************
 **  main.c (S. Weaver)
 **********************************************************/  

#include "ite.h"

extern char tracer_tmp_filename[256];
extern BDDNodeStruct **original_functions;

void bdd_bdd_free_pools();
void bdd_circuit_free();
void wvfSolve();
void Verify_NoSolver(Tracer *tracer);
void Verify_Solver(Tracer *tracer);

int ite_pre_init();
int ite_init();
void ite_free(Tracer *tracer);
int ite_io_init();
void ite_io_free();
int ite_preprocessing();

int check_expected_result(int result);

int 
main(int argc, char *argv[])
{
   Tracer * tracer = NULL;
   int ret;
	
   ret = ite_pre_init();
   if (ret != NO_ERROR) goto ExitNormal;
	
   ret = params_parse_cmd_line(argc, argv);
   if (ret != NO_ERROR) goto ExitNormal;
	
   if (DEBUG_LVL == 1) d1_printf2("%s ", inputfile);
	
   ret = ite_init();
   if (ret != NO_ERROR) goto ExitNormal;
	
   ret = ite_io_init();
   if (ret != NO_ERROR) goto ExitNormal;
	
   /* autodetect the input format  */
   formatin = getformat (finputfile);
	
   /* read input file */
   ret = read_input(formatin, formatout, tracer);
   if (ret != NO_ERROR) goto ExitNormal;


   /* preprocessing */
   ret = ite_preprocessing();
   if (ret == TRIV_SAT || ret == TRIV_UNSAT) goto ExitNormal;

   {
      /*
       * reversing dependency if requested
       */
      if (reverse_independant_dependant)
      {
         for (int x = 0; x < numinp + 1; x++)
         {
            if (independantVars[x] == 1)
               independantVars[x] = 0;
            else if (independantVars[x] == 0)
               independantVars[x] = 1;
         }
      }

      if (clear_dependance)
      {
         for (int x = 0; x < numinp + 1; x++)
            if (independantVars[x] == 0)
               independantVars[x] = 1;
      }
   }
   /* solver or conversion */


	switch (formatout) {
    case 'n': break;
    case 'b': ret = solve(tracer); break;
    case 'w': ret = walkSolve(); break;
    case 'm': wvfSolve(); break;
    default: write_output(formatout, tracer);   
             ret = CONV_OUTPUT;
   }

ExitNormal:

   if (s_expected_result[0]) {
      ret = check_expected_result(ret);
   }

   if (ite_counters[BDD_NODE_FIND]==0) ite_counters[BDD_NODE_FIND] = 1;
   d2_printf4("BDD Lookup Statistic: %ld/%ld (%f hit rate)\n", 
         (long)(ite_counters[BDD_NODE_FIND] - ite_counters[BDD_NODE_NEW]), 
         (long)(ite_counters[BDD_NODE_FIND]),
         1.0 * (ite_counters[BDD_NODE_FIND] - ite_counters[BDD_NODE_NEW]) / ite_counters[BDD_NODE_FIND]);
   d2_printf4("BDD Hash Table Lookup Statistic: %ld/%ld (%f steps)\n", 
         (long)(ite_counters[BDD_NODE_STEPS]),
         (long)(ite_counters[BDD_NODE_FIND]),
         1.0 * ite_counters[BDD_NODE_STEPS] / ite_counters[BDD_NODE_FIND]);
   d2_printf4("BDD Hash Table Storage Statistic: %ld/%ld (%f nodes taken)\n", 
         (long)(ite_counters[BDD_NODE_NEW]),
         (long)((1<<(numBuckets+sizeBuckets))-1),
         1.0 * ite_counters[BDD_NODE_NEW] / (long)((1<<(numBuckets+sizeBuckets))-1));

   char result_string[64];
   switch(ret) {
    case TRIV_SAT: 
       {
          strcpy(result_string, "Satisfiable");
          Verify_NoSolver(tracer);
       };
       break;
    case SOLV_SAT: {
       if (ite_counters[NUM_SOLUTIONS] > 1) {
          sprintf(result_string, "Satisfiable(%lld)", ite_counters[NUM_SOLUTIONS]);
       } else {
          strcpy(result_string, "Satisfiable");
       }
       Verify_Solver(tracer);
    } 
       break;
    case TRIV_UNSAT: 
    case SOLV_UNSAT: strcpy(result_string, "Unsatisfiable"); break;
    case SOLV_UNKNOWN: strcpy(result_string, "Unknown Result"); break;
    case CONV_OUTPUT: strcpy(result_string, "Conversion"); break;
    default: sprintf(result_string, "Error(%d)", ret); break;
   }

   ite_io_free();
   ite_free(tracer);

   ite_counters_f[RUNNING_TIME] = get_runtime();
   if (DEBUG_LVL == 1) {
      d1_printf3("%s %lld ", result_string, ite_counters[NO_ERROR]);
      d1_printf4("%.3fs %.3fs %.3fs\n", 
            ite_counters_f[RUNNING_TIME], 
            ite_counters_f[PREPROC_TIME], 
            ite_counters_f[BRANCHER_TIME]);
   } else {
      if (DEBUG_LVL > 0) {
         d0_printf2("%s\n", result_string);
         d2_printf2("Total Time: %4.3f \n", get_runtime());
      } else {
         printf("%s\n", result_string);
      }
   }

   if (ret == SOLV_ERROR) return 1;
   return 0;
}

int 
ite_pre_init()
{
   int i;
   for (i=0;i<MAX_COUNTER;i++) ite_counters[i] = 0;
   for (i=0;i<MAX_COUNTER_F;i++) ite_counters_f[i] = 0;

   return NO_ERROR;
}

int 
ite_init()
{
   d9_printf1("ite_init\n");
   // bdd_init(); -- moved to bdd_circuit_init
   return NO_ERROR;
}

void 
ite_free(Tracer *tracer)
{
   d9_printf1("ite_free\n");
   if (tracer != NULL) {
      delete tracer;
      d9_printf2("Removing file: %s\n", tracer_tmp_filename);
      unlink (tracer_tmp_filename);
   }

   delete [] variablelist;
   variablelist = NULL;
   delete [] original_functions;
   original_functions = NULL;

   bdd_bdd_free_pools();
   bdd_circuit_free();
}

void 
ite_io_free()
{
   d9_printf1("ite_io_free\n");
   if (finputfile && finputfile != stdin) fclose(finputfile);
   if (foutputfile && foutputfile != stdout && foutputfile != stderr) fclose(foutputfile);
   //if (fresultfile != stdin && fresultfile) fclose(fresultfile);
}

int 
ite_io_init()
{
   d9_printf1("ite_io_init\n");
   /* 
    * open the input file 
    */

   if (!strcmp(inputfile, "-")) { d2_printf1("Reading standard input....\n"); }
   else { d2_printf2("Reading File %s ....\n", inputfile); }


   if (check_gzip(inputfile)) {
      d2_printf1("gzip file -- using zread\n");
      finputfile = zread(inputfile);
   }
   else
      if (!strcmp(inputfile, "-")) {
         finputfile = stdin;
      }
      else {
         finputfile = fopen(inputfile, "r");
      }

   if (!finputfile) { 
      dE_printf2("Can't open the input file: %s\n", inputfile);
      return ERR_IO_INIT;
   } else d9_printf2("Input file opened: %s\n", inputfile);

   /* 
    * open the output file 
    */

   if (!strcmp(outputfile, "-")) foutputfile = stdout;
   else {
      struct stat buf;
      if (stat(outputfile, &buf) == 0) {
         dE_printf2("Error: File %s exists\n", outputfile);
         return ERR_IO_INIT;
      }
      foutputfile = fopen(outputfile, "w");
   }

   if (!foutputfile) { 
      dE_printf2("Can't open the output file: %s\n", outputfile);
      return ERR_IO_INIT;
   } else d9_printf2("Output file opened: %s\n", outputfile);

   /* 
    * open the result file
    */
   /*
    if (input_result_filename[0]) {
    fresultfile = fopen(input_result_filename, "r");

    if (!fresultfile) {
    dE_printf2("Can't open the result filename: %s\n", input_result_filename);
    return ERR_IO_INIT;
    } else
    d9_printf2("Result file opened: %s\n", input_result_filename);
    }
    */
   return NO_ERROR;
}

int 
ite_preprocessing()
{
   d9_printf1("ite_preprocessing\n");
   int ret=0, prep_ret=0;
   /*
    * Preprocessing - start
    */
   prep_ret = Init_Preprocessing();

   /*
    * read partial/full assignment file
    */ 
   /*
    extern varinfo * variablelist;
    extern char input_result_filename[128];
    int read_input_result(FILE *, Tracer *tracer, varinfo*);
    FILE *fresultfile=NULL;

    if (input_result_filename[0]) {
    if (read_input_result(fresultfile, tracer, variablelist)) goto ExitNormal;
    else {
    d9_printf1("Finished reading input result\n");
   //ret = MakeInfs(variablelist);
   //ret = DoInfs();
   }
   }
   else d9_printf1("Result input file not specified\n");
    */

   if (prep_ret == PREP_NO_CHANGE || prep_ret == PREP_CHANGED) {
      prep_ret = Preprocessor (variablelist);
   }

   ret = Finish_Preprocessing();

   if (prep_ret == TRIV_SAT || prep_ret == TRIV_UNSAT) {
      ret = prep_ret;
   }
   /*
    * Preprocessing - end
    */
   d9_printf2("ite_preprocessing done: %d\n", ret);
   return ret;
}

int 
check_expected_result(int result)
{
   switch (result) {
    case TRIV_SAT: 
       if (strcmp(s_expected_result, "TRIV_SAT") && 
             strcmp(s_expected_result, "SAT")) return SOLV_ERROR;
       break;
    case TRIV_UNSAT: 
       if (strcmp(s_expected_result, "TRIV_UNSAT") && 
             strcmp(s_expected_result, "UNSAT")) return SOLV_ERROR;
       break;
    case SOLV_SAT:  
       if (strcmp(s_expected_result, "SOLV_SAT") && 
             strcmp(s_expected_result, "SAT")) return SOLV_ERROR;
       break;
    case SOLV_UNSAT: 
       if (strcmp(s_expected_result, "SOLV_UNSAT") && 
             strcmp(s_expected_result, "UNSAT")) return SOLV_ERROR;
       break;
    default: /* can't verify the result */
       dE_printf1("Can't check the result against the expected result\n");
       break;
   }	
   return result;
};
