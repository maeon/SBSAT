#include <stdio.h>
#include <sys/time.h>
#include "ite.h"

/*
 * van der Warden Number problem:
 * 
 * Consider the small example: find an arrangement of positive 
 * integers from 1 to n, into one and only one of our k buckets, 
 * such that there is *no* arithmetic progression of length p.
 * 
 * vdW(4,5)=100 (so n=100, k=4, p=5)
 * 
 * Model this with 4*100 boolean variables, 
 * vars 1 - 100 represent the integers existence in bucket 1
 * vars 101 - 200 represent the integers existence in bucket 2
 * ..
 * vars 301 - 400 represent the integers existence in bucket 4
 * 
 * * a variable is true => the integer is in the bucket
 * 
 * We need three types of cnf clauses (or just two when using 
 * our new input format):
 * 1. every integer is in one bucket
 * 
 * so clauses
 * 1 101 201 301 0
 * 2 102 202 302 0
 * ...
 * 100 200 300 400 0
 * 
 * 2. but in only one bucket (so we have to pairwise prevent 
 * this from happening in two (or more) buckets
 * 
 * // for bucket one
 * -1 -101 0
 * -1 -201 0
 * -1 -301 0
 * -101 -201 0
 * -101 -301 0
 * -201 -301 0
 * 
 * // for bucket two
 * -2 -102 0
 * -2 -202 0
 * -2 -302 0
 * -102 -202 0
 * -102 -302 0
 * -202 -302 0
 * 
 * etc
 * 
 * 3. prevent any arithmetic progression of length 5
 * 
 * for each bucket
 * for each possible progression of length 5
 * 
 * -1 -2 -3 -4 -5 0
 * -1 -3 -5 -7 -9 0
 * -1 -4 -7 -10 -13 0
 * ... 
 * -1 -21 -41 -61 -81 0
 * -1 -22 -43 -64 -85 0
 * -1 -23 -45 -67 -89 0
 * -1 -24 -47 -70 -93 0
 * -1 -25 -49 -73 -97 0
 * 
 * note: we don't need -1 -26 -51 -76 -101 0 since there is no 
 * integer 101 for placement
 * 
 * We need to do this for every progression of length 5, for every bucket
 * 
 * Our new format will eliminate the first two types of cnf 
 * clauses and replace it with a nice compact representation. 
 * 
 * If you can think of a way to condense #3 type clauses into a 
 * nice state machine, I would love to hear it.
 */
#define CNF  0
#define XCNF 1
#define ITE  2
#define CNF2 3
#define MINXCNF 4
#define CNF2MK 5

char name[128];
int formula_type = XCNF;

char *var(int n, int x, int k) { 
   int i = k*n+x;
   if (formula_type != ITE)
      sprintf(name, "%d", i);
   else
      sprintf(name, "b%dv%d", k, x);
   return name;
}

void vanDerWaerden(char *vdw_type, int n, int k, int p) {
   if (!strcmp(vdw_type, "cnf")) formula_type = CNF; else 
   if (!strcmp(vdw_type, "xcnf")) formula_type = XCNF; else 
   if (!strcmp(vdw_type, "cnf2")) formula_type = CNF2; else 
   if (!strcmp(vdw_type, "cnf2mk")) formula_type = CNF2MK; else 
   if (!strcmp(vdw_type, "ite")) formula_type = ITE; else 
   if (!strcmp(vdw_type, "minxcnf")) formula_type = MINXCNF; else {
      fprintf(stderr, "Unknown vdw formula type\n");
      exit(1);
   }

   // clauses max step = (n-1)/(p-1)  [n is 1 based and k is 1 based]
   // clauses with step 1 = n-p+1
   // clauses with step 2 = n-p*2+2
   // clauses with step 3 = n-p*3+3
   // clauses with step 4 = n-p*4+4
   // prog_sum = (1+max_step)*max_step/2
   // number of regressions = max_step * n - max_step * p * prog_sum + prog_sum
   // and make it for each bucket = k*number_of_regressions
   int max_step = (n-1)/(p-1); // max sure this is integer
   int prog_sum = ((1+max_step)*max_step)/2;
   int clauses = n+k*(max_step*n-p*prog_sum+prog_sum);
   int clause_count = 0;
   int sym = 0;
   int sym_clauses = 0;
   switch (formula_type) {
    case CNF: 
      clauses += n*k*(k-1)/2;
      fprintf(stdout, "p cnf %d %d\n", n*k, clauses);
      break;
    case XCNF: 
      fprintf(stdout, "p cnf %d %d\n", n*k, clauses);
      break;
    case ITE: 
      fprintf(stdout, "p bdd %d %d\n", n*k, 1+clauses);
      //fprintf(stdout, "initial_branch(");
      for(int x=1; x<=n; x++)
         fprintf(stdout, "%s ", var(n, x, 0));
      fprintf(stdout, ")\n");
      break;
    case CNF2MK:
      sym = n/(p-1); //mk version
      if (sym*(p-1) != n) { fprintf(stderr, "Can't do it properly\n"); exit(1); }
      sym_clauses = (sym)*(p-2)*2;
      sym_clauses += - 2*(p-2) - ((sym-1)/11)*2*(p-2);
    case CNF2:
      if (k != 2) {
         fprintf(stderr, "Can't use CNF2 for any other k but 2\n");
         exit(1);
      }
      clauses -= n;
      fprintf(stdout, "p cnf %d %d\n", n*k, clauses+sym_clauses);
      break;
    case MINXCNF:
      clauses -= n;
      clauses /= k;
      clauses++;
      fprintf(stdout, "p cnf %d %d\n", n, clauses);
      break;
    default: 
      exit(1);
   }

   if (formula_type == CNF || formula_type == XCNF)
      fprintf(stdout, "c max_step %d\n", max_step);

   // Problem:
   // n integer, k buckets, n*k variables

   // 1. 
   if (formula_type == CNF || formula_type == XCNF)
      fprintf(stdout, "c every integer only in one bucket\n");
   for(int x = 1; x <= n; x++) {
      int bucket;
      switch (formula_type) {
       case CNF: {
          // at least one bucket
          fprintf(stdout, "%s ", var(n, x, 0));
          for(bucket = 1; bucket < k; bucket++)
             fprintf(stdout, "%s ", var(n, x, bucket));
          fprintf(stdout, "0\n");
          clause_count++;
          // at most one bucket
          for(bucket = 0; bucket < k; bucket++)
             for (int bucket2=bucket+1; bucket2 < k; bucket2++) {
                fprintf(stdout, "-%s ", var(n, x, bucket));
                fprintf(stdout, "-%s 0\n", var(n, x, bucket2));
                clause_count++;
             }
       } break;
       case XCNF: {
         fprintf(stdout, "#1 [ %s ", var(n, x, 0));
         for(bucket = 1; bucket < k; bucket++)
            fprintf(stdout, "%s ", var(n, x, bucket));
         fprintf(stdout, "] 1\n");
         clause_count++;
       } break;
       case ITE: {
          // at least one bucket
          fprintf(stdout, "* AND%d( OR%d(%s ", 1+k*(k-1)/2, k, var(n, x, 0));
          for(bucket = 1; bucket < k; bucket++)
             fprintf(stdout, "%s ", var(n, x, bucket));
          clause_count++;
          fprintf(stdout, ") ");
          // at most one bucket
          for(bucket = 0; bucket < k; bucket++) {
             for (int bucket2=bucket+1; bucket2 < k; bucket2++) {
                fprintf(stdout, "OR(-%s  ", var(n, x, bucket));
                fprintf(stdout, "-%s) ", var(n, x, bucket2));
                clause_count++;
             }
          }
          fprintf(stdout, ")\n");
       } break;
       case MINXCNF: {
          /* nothing */
      } break;
      }
  }
	
   if (formula_type == CNF || formula_type == XCNF) {
      fprintf(stdout, "c prevent any arithmetic progression of length %d for every bucket %d\n", p, k);
      for(int bucket=0; bucket<k; bucket++) {
         for(int num = 1; num <= n; num++) {
            for(int step = 1; 1; step++) {
               if (step * (p-1) + num > n) break;
               int base = num;
               for(int z = 0; z < p; z++) {
                  fprintf(stdout, "-%s ", var(n, base, bucket));
                  base+=step;
               }
               fprintf(stdout, "0\n");
               clause_count++;
            }
         }
      }
      if (clause_count != clauses) {
         fprintf(stderr, "======================== Problem\n");
      }
   } else if (formula_type == ITE) {
      for(int bucket=0; bucket<k; bucket++) {
         for(int num = 1; num <= n; num++) {
//#define MK_TEST
#ifdef MK_TEST
            if ((n-num)/(p-1) == 0) continue;
            fprintf(stdout, "* AND%d(", (n-num)/(p-1));
#endif
            for(int step = 1; 1; step++) {
               if (step * (p-1) + num > n) break;
               int base = num;
#ifdef MK_TEST
               fprintf(stdout, " OR%d( ", p);
#else
               fprintf(stdout, "* OR%d( ", p);
#endif
               for(int z = 0; z < p; z++) {
                  fprintf(stdout, "-%s ", var(n, base, bucket));
                  base+=step;
               }
#ifdef MK_TEST
               fprintf(stdout, ")");
#else
               fprintf(stdout, ")\n");
#endif
               clause_count++;
            }
#ifdef MK_TEST
            fprintf(stdout, ")\n");
#endif
         }
      }
   } else if (formula_type == CNF2 || formula_type == CNF2MK) {
      fprintf(stdout, "c prevent any arithmetic progression of length %d for every bucket %d\n", p, k);
      for(int num = 1; num <= n; num++) {
         for(int step = 1; 1; step++) {
            if (step * (p-1) + num > n) break;
            int base = num;
            for(int z = 0; z < p; z++) {
               fprintf(stdout, "-%s ", var(n, base, 0));
               base+=step;
            }
            fprintf(stdout, "0\n");
            clause_count++;
            base = num;
            for(int z = 0; z < p; z++) {
               fprintf(stdout, "%s ", var(n, base, 0));
               base+=step;
            }
            fprintf(stdout, "0\n");
            clause_count++;
         }
      }
      if (clause_count != clauses) {
         fprintf(stderr, "======================== Problem\n");
      }
      if (formula_type == CNF2MK) {
         // add symetry
         for(int i=1;i<=sym;i++) {
            if ((i-1)%11 == 0) continue;
            for(int j=1;j<(p-1);j++) {
               int eq_var = j*sym+i;
               fprintf(stdout, "-%s ", var(n, i, 0));
               fprintf(stdout, "%s 0\n", var(n, eq_var, 0)); 
               fprintf(stdout, "%s ", var(n, i, 0));
               fprintf(stdout, "-%s 0\n", var(n, eq_var, 0)); 
            }
         }
      }
   } else if (formula_type == MINXCNF) {
      fprintf(stdout, "c prevent any arithmetic progression of length %d for every bucket %d\n", p, k);
      fprintf(stdout, "#1 [ ");
      for(int y = 1; y <= n; y++)
         fprintf(stdout, "%s ", var(n, y, 0));
      fprintf(stdout, "] %d\n", n/2);
      clause_count++;
      for(int num = 1; num <= n; num++) {
         for(int step = 1; 1; step++) {
            if (step * (p-1) + num > n) break;
            int base = num;
            for(int z = 0; z < p; z++) {
               fprintf(stdout, "-%s ", var(n, base, 0));
               base+=step;
            }
            fprintf(stdout, "0\n");
            clause_count++;
         }
      }
      if (clause_count != clauses) {
         fprintf(stderr, "======================== Problem (%d != %d)\n",
               clause_count, clauses);
      }
   }
}

