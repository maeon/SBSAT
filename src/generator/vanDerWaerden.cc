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

void vanDerWaerden(int n, int k, int p) {
	//fprintf(stdout, "p cnf %d %d\n", n*k, n+(n*((n-1)/(p-1))));
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
	fprintf(stdout, "p cnf %d %d\n", n*k, n+k*(max_step*n-p*prog_sum+prog_sum));
   fprintf(stdout, "c max_step %d\n", max_step);

   // Problem:
   // n integer, k buckets, n*k variables

   // 1. 
   fprintf(stdout, "c every integer only in one bucket\n");
   for(int x = 1; x <= n; x++) {
      int y;
      if (1) {
         fprintf(stdout, "#1 [ %d ", x);
         for(y = 1; y < k; y++)
            fprintf(stdout, "%d ", (y*n)+x);
         fprintf(stdout, "] 1\n");
      } else {
         // at least one bucket
         fprintf(stdout, "%d ", x);
         for(y = 1; y < k; y++)
            fprintf(stdout, "%d ", (y*n)+x);
         fprintf(stdout, "0\n", (y*n)+x);
         // at most one bucket
         for(y = 0; y < k; y++)
            for (int z=y+1; z < k; z++)
               fprintf(stdout, "-%d -%d 0\n", (y*n)+x, (z*n)+x);
      }
   }
	
   fprintf(stdout, "c prevent any arithmetic progression of length %d for every bucket %d\n", p, k);
   for(int bucket=0; bucket<k; bucket++) {
      for(int num = 1; num <= n; num++) {
         for(int step = 1; 1; step++) {
            if (step * (p-1) + num > n) break;
            int base = num;
            for(int z = 0; z < p; z++) {
               fprintf(stdout, "-%d ", base+bucket*n);
               base+=step;
            }
            fprintf(stdout, "0\n");
         }
      }
   }
}

