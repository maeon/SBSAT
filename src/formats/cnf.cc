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
/**********************************************************************
 *  cnf.c (S. Weaver)
 *  Routines for converting from CNF to BDD and from DNF to CNF
 **********************************************************************/  

#include "ite.h"

int zecc_limit;
int *zecc_arr;

//If you want character support for strings and things, look in markbdd.c and copy that...
char getNextSymbol_CNF (char macros[20], int &intnum) {
	char integers[20];
	int i = 0;
	char p = 0;
	while(1) {
		if((p = fgetc(finputfile)) == EOF) {
			fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
			exit (1);
		}
      if(p == 'c') {
			while(p != '\n') {
				if((p = fgetc(finputfile)) == EOF) {
					fprintf(stderr, "\nUnexpected EOF...exiting\n");
					exit(1);
				}
			}
			if ((p = fgetc(finputfile)) == EOF) {
				fprintf(stderr, "\nUnexpected EOF...exiting\n");
				exit(1);
			}
			ungetc(p, finputfile);
			continue;
		}
      if(p == '#') {
			if((p = fgetc(finputfile)) == EOF) {
				fprintf(stderr, "\nUnexpected EOF...exiting\n");
				exit(1);
			}
			if(((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))) {
				while((p != '(') && (p != ' ') && (p != '\n') && (p != ';')) {
					macros[i] = p;
					i++;
					if((p = fgetc(finputfile)) == EOF) {
						fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
						exit (1);
					}
				}
				ungetc(p, finputfile);
				macros[i] = 0;
				return 'm';
			} else {
				ungetc(p, finputfile);
				return '#';
			}
		}	
		if(((p >= '0') && (p <= '9')) || (p == '-')) {
			i = 0;
			while(((p >= '0') && (p <= '9')) || (p == '-')) {
				integers[i] = p;
				i++;
				if((p = fgetc(finputfile)) == EOF) {
					break;
					//	fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
					//	exit(1);
				}
			}
			ungetc(p, finputfile);
			integers[i] = 0;
			intnum = atoi(integers);
			return 'i';
		}
	}
}

store *getMinMax() {
	int i = 0, min, max;
	char macros[20];
	char p;
	
//	if((p = fgetc(finputfile)) == EOF) {
//		fprintf(stderr, "\nUnexpected EOF...exiting\n");
//		exit (1);
//	}

	char order = getNextSymbol_CNF (macros, min);
	if(order != 'i' || min < 0) {
		fprintf(stderr, "Error looking for min while parsing CNF input (%s)...exiting\n", macros);
		exit(1);
	}
	
	if((p = fgetc(finputfile)) == EOF) {
		fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
		exit (1);
	}

	while(p != '[') {
		if(p != ' ') {
			fprintf(stderr, "\nExpecting '[' after min:%d, found (%c)...exiting\n", min, p);
			exit (1);
		}
		if((p = fgetc(finputfile)) == EOF) {
			fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
			exit (1);
		}
	}
	if((p = fgetc(finputfile)) == EOF) {
		fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
		exit (1);
	}
	
	int x = 0;
	while(p != ']') {
		i = 0;
		while((p >= '0') && (p<= '9')) {
			macros[i] = p;
			i++;
			if((p = fgetc(finputfile)) == EOF) {
				fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
				exit (1);
			}
		}
		if(p!=' ' && p!=']') {
			fprintf(stderr, "\nExpecting ']', found (%c)...exiting\n", p);
			exit (1);
		}
		if((p = fgetc(finputfile)) == EOF) {
			fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
			exit (1);
		}
		
		if(i!=0) {
			macros[i] = 0;
			tempint[x] = atoi(macros);
			x++;
		}
	}
	int num_vars = x;
	order = getNextSymbol_CNF (macros, max);	
	if(order != 'i' || max < 0) {
		fprintf(stderr, "Error looking for max while parsing CNF input (%s)...exiting\n", macros);
		exit(1);
	}
	if(max < min) {
		fprintf(stderr, "Max:%d must be greater than or equal to Min:%d...exiting\n", max, min);
		exit(1);
	}
	store *min_max = new store;
	min_max->length = num_vars;
	min_max->num = new int[num_vars];
	for(x = 0; x < num_vars; x++) {
		min_max->num[x] = tempint[x];
		if(abs(tempint[x]) > numinp) {
			fprintf(stderr, "Variable in input file is greater than allowed:%d...exiting\n", numinp);
			exit(1);				
		}
	}
	min_max->dag = min;
	min_max->andor = max;
	return min_max;
}

//Function CNF_to_BDD takes CNF input from STDIN and returns a list
//of pointers to the tranlated BDDs
void CNF_to_BDD(int cnf)
{
//	int tempint[5000]; //global
   tempint = new int[5000];
	char macros[20], order;
	int intnum;
	long out, count, num;
	long y, z, i, j, temp;

	
	FILE *fin;
	fin = fopen ("aa", "rb");
//	zecc_arr = new int[1000];
	zecc_limit = 0;
//	do{
//		fscanf(fin, "%d\n", &zecc_arr[zecc_limit++]);
//	}while(zecc_arr[zecc_limit++]!=0);
//	fclose (fin);
	
	//Get number of inputs and number of outputs from
	//the header of the CNF file
	fscanf(finputfile, "%ld %ld\n", &numinp, &numout);
	numinp+=2;
	store *integers = new store[numout+2];
	store *old_integers = new store[numout+2];

	d2_printf1("Storing Inputs");

	int old_numout = numout;
	//Get and store the CNF clauses from STDIN
	int num_minmax = 0;
	for(long x = 1; x < numout + 1; x++) {
      y = -1;
      do {
			y++;
			order = getNextSymbol_CNF(macros, intnum);
			if(order == '#') {
				store *temp = getMinMax();
				integers[x].num = temp->num;
				integers[x].length = temp->length;
				integers[x].dag = temp->dag;
				integers[x].andor = temp->andor;
				delete temp;
				old_integers[x] = integers[x];
				num_minmax++;
				break;
			} else {
				if(order != 'i') {
					fprintf(stderr, "Error while parsing CNF input:%ld...exiting\n", x);
					exit(1);
				}
			}
			tempint[y] = intnum;
		} while(tempint[y] != 0);
		if(order == '#') {
			order = 0;
			continue;
		}
		integers[x].dag = -1;
		if(y==0) {x--; numout--; continue; }
		integers[x].num = new int[y + 1];
      for(i = 0; i < y + 1; i++) {
			integers[x].num[i] = tempint[i];
			if(abs(tempint[i]) > numinp) {
				fprintf(stderr, "Variable in input file is greater than allowed:%d...exiting\n", numinp);
				exit(1);				
			}
			//fprintf(stderr, "%d ", integers[x].num[i]);
		}
      
      //fprintf(stderr, "\n");
		integers[x].length = y;
		old_integers[x] = integers[x];
	}
	minmax *min_max_store = new minmax[num_minmax+1];
	if(num_minmax > 0) {
		int y = 1;
		num_minmax = 0;
		for(long x = 1; x < numout + 1; x++) {
			integers[y] = integers[x];
			y++;
			if(integers[x].dag != -1) {
				min_max_store[num_minmax].num = integers[x].num;
				min_max_store[num_minmax].length = integers[x].length;
				min_max_store[num_minmax].min = integers[x].dag;
				min_max_store[num_minmax].max = integers[x].andor;
				num_minmax++; //this is a little silly, num_minmax = x-y
				y--;
			}
		}
		numout = y-1;
	}
	
	d2_printf2("numinp: %ld\n", numinp);
	if(DO_CLUSTER) {
      int *twopos_temp = (int *)calloc(numinp+1, sizeof(int));
      int *twoneg_temp = (int *)calloc(numinp+1, sizeof(int));
      int *greaterpos_temp = (int *)calloc(numinp+1, sizeof(int));
      int *greaterneg_temp = (int *)calloc(numinp+1, sizeof(int));
      for(long x = 1; x < numout + 1; x++) {
			if(integers[x].length == 2) {
				for(y = 0; y < 2; y++) {
					if(integers[x].num[y] > 0)
					  twopos_temp[integers[x].num[y]]++;
					else
					  twoneg_temp[-integers[x].num[y]]++;
				}
			}
			else if(integers[x].length > 2) {
				for(y = 0; integers[x].num[y] != 0; y++) {
					if(integers[x].num[y] > 0)
					  greaterpos_temp[integers[x].num[y]]++;
					else
					  greaterneg_temp[-integers[x].num[y]]++;
				}
			}
		}
      store *two_pos = (store *)calloc(numinp+1, sizeof(store));
      store *two_neg = (store *)calloc(numinp+1, sizeof(store));
      
		//two_pos and two_neg are lists that contain all the clauses
		//that are of length 2. two_pos contains every 2 variable clause
		//that has a positive variable, two_neg contains every 2
		//variable clause that has a negative variable. There will most likely
		//be some overlaps in the variable storing.
		//EX)
		//p cnf 3 3
		//2 3 0
		//-2 -3 0
		//-2 3 0
		//
		//two_pos will point to (2:3)   and (-2:3)
		//two_neg will point to (-2:-3) and (-2:3)
		store *greater_pos = (store *)calloc(numinp+1, sizeof(store));
      store *greater_neg = (store *)calloc(numinp+1, sizeof(store));
      
		//greater_pos and greater_neg are similar to two_pos and two_neg
		//except greater_pos and greater_neg contain all clauses with more
		//than 2 variables in length.
		d2_printf1("Done Scanning");
      num = 0;
      temp = 0;
      
		//Storing appropriate array sizes...helps with memory!
		for(long x = 1; x < numinp + 1; x++) {
			two_pos[x].num = (int *)calloc(twopos_temp[x]+1, sizeof(int));
			two_neg[x].num = (int *)calloc(twoneg_temp[x]+1, sizeof(int));
			greater_pos[x].num = (int *)calloc(greaterpos_temp[x]+1, sizeof(int));
			greater_neg[x].num = (int *)calloc(greaterneg_temp[x]+1, sizeof(int));
		}
      free(twopos_temp);
      free(twoneg_temp);
      free(greaterpos_temp);
      free(greaterneg_temp);
      
		//This is where two_pos, two_neg, greater_pos, and greater_neg are
		//filled with clauses
		for(long x = 1; x < numout+1; x++) {
			count = 0;
			if(integers[x].length == 2) {
				if(integers[x].num[0] > 0) {
					out = 0;
					for(y = 0; two_pos[integers[x].num[0]].num[y] != 0; y++) {
						//if there is a duplicate 2 variable clause, throw it out!
						if(((integers[two_pos[integers[x].num[0]].num[y]].num[0] == integers[x].num[0]) 
						  &&(integers[two_pos[integers[x].num[0]].num[y]].num[1] ==	integers[x].num[1]))
						 ||((integers[two_pos[integers[x].num[0]].num[y]].num[0] ==	integers[x].num[1])
						  &&(integers[two_pos[integers[x].num[0]].num[y]].num[1] == integers[x].num[0])))	
						  {
							  out = 1;
							  integers[x].length = 0;
						  }
					}
					if(out == 0) {
						//if this is a unique clase then keep it
						two_pos[integers[x].num[0]].num[y] = x;
						two_pos[integers[x].num[0]].num[y + 1] = 0;
					}
				} else {
					out = 0;
					for(y = 0; two_neg[-integers[x].num[0]].num[y] != 0; y++) {
						//if there is a duplicate 2 variable clause, throw it out!
						if(((integers[two_neg[-integers[x].num[0]].num[y]].num[0] == integers[x].num[0])
					     &&(integers[two_neg[-integers[x].num[0]].num[y]].num[1] == integers[x].num[1]))
						 ||((integers[two_neg[-integers[x].num[0]].num[y]].num[0] == integers[x].num[1])
						  &&(integers[two_neg[-integers[x].num[0]].num[y]].num[1] == integers[x].num[0])))
						  {
							  out = 1;
							  integers[x].length = 0;
						  }
					}
					if(out == 0) {
						//if this is a unique clase then keep it
						two_neg[-integers[x].num[0]].num[y] = x;
						two_neg[-integers[x].num[0]].num[y + 1] = 0;
					}
				}
				if(integers[x].num[1] > 0) {
					out = 0;
					for(y = 0; two_pos[integers[x].num[1]].num[y] != 0; y++) {
						//if there is a duplicate 2 variable clause, throw it out!
						if(((integers[two_pos[integers[x].num[1]].num[y]].num[0] == integers[x].num[0])
						  &&(integers[two_pos[integers[x].num[1]].num[y]].num[1] == integers[x].num[1]))
						 ||((integers[two_pos[integers[x].num[1]].num[y]].num[0] == integers[x].num[1]) 
						  &&(integers[two_pos[integers[x].num[1]].num[y]].num[1] == integers[x].num[0])))
						  {
							  out = 1;
							  integers[x].length = 0;
						  }
					}
					if(out == 0) {
						//if this is a unique clase then keep it
						two_pos[integers[x].num[1]].num[y] = x;
						two_pos[integers[x].num[1]].num[y + 1] = 0;
					}
				} else {
					out = 0;
					for(y = 0; two_neg[-integers[x].num[1]].num[y] != 0; y++) {
						//if there is a duplicate 2 variable clause, throw it out!
						if(((integers[two_neg[-integers[x].num[1]].num[y]].num[0] == integers[x].num[0])
						  &&(integers[two_neg[-integers[x].num[1]].num[y]].num[1] == integers[x].num[1]))
						 ||((integers[two_neg[-integers[x].num[1]].num[y]].num[0] == integers[x].num[1])
						  &&(integers[two_neg[-integers[x].num[1]].num[y]].num[1] == integers[x].num[0])))
						  {
							  out = 1;
							  integers[x].length = 0;
						  }
					}
					if(out == 0) {
						//if this is a unique clase then keep it
						two_neg[-integers[x].num[1]].num[y] = x;
						two_neg[-integers[x].num[1]].num[y + 1] = 0;
					}
				}
			} else if(integers[x].length > 2) {
				for(z = 0; integers[x].num[z] != 0; z++) {
					if(integers[x].num[z] > 0) {
						for(y = 0; greater_pos[integers[x].num[z]].num[y] != 0; y++) {}
						greater_pos[integers[x].num[z]].num[y] = x;
						greater_pos[integers[x].num[z]].num[y + 1] = 0;
					} else {
						for(y = 0; greater_neg[-integers[x].num[z]].num[y] != 0; y++) {}
						greater_neg[-integers[x].num[z]].num[y] = x;
						greater_neg[-integers[x].num[z]].num[y + 1] = 0;
					}
				}
			}
		}
      num = 0;
      d2_printf1("Starting Search");
      
		//ok...this is where the and= and or= are sorted out.
		//I'll have to make a good explaination later
		//cause this was hard to work out.
      int and_or_do = 1;
      while(and_or_do) {
			and_or_do = 0;
			for(long x = 1; x < numinp+1; x++) {
				if(two_pos[x].num[0] > 0) {
					out = 0;
					for(z = 0; (greater_neg[x].num[z] != 0) && (out != 1); z++) {
						count = 0;
						for(y = 0; integers[greater_neg[x].num[z]].num[y] != 0; y++) {
							if(integers[greater_neg[x].num[z]].dag == -1) {
								for(i = 0; two_pos[x].num[i] != 0; i++) {
									if(integers[two_pos[x].num[i]].dag == 2)
									  continue;
									if(((-integers[greater_neg[x].num[z]].num[y] == integers[two_pos[x].num[i]].num[0])
										&&(integers[two_pos[x].num[i]].num[0] != x))
									 ||((-integers[greater_neg[x].num[z]].num[y] == integers[two_pos[x].num[i]].num[1])
										&&(integers[two_pos[x].num[i]].num[1] != x)))
									  {
										  integers[two_pos[x].num[i]].dag = 2;
										  count++;
									  }
								}
							}
						}
						if(count == y - 1) {
							for(i = 0; two_pos[x].num[i] != 0; i++) {
								if(integers[two_pos[x].num[i]].dag == 2)
								  integers[two_pos[x].num[i]].length = 0;
							}
							and_or_do = 1;
							integers[greater_neg[x].num[z]].dag = 1;
//							for(i = 0; -integers[greater_neg[x].num[z]].num[i] != x; i++) {}
//							integers[greater_neg[x].num[z]].andor = i;
							for(i = 0; integers[greater_neg[x].num[z]].num[i] != 0; i++) {}
							integers[greater_neg[x].num[z]].length = i;
							integers[greater_neg[x].num[z]].andor = -x;
							out = 1;
							num++;
						}
						for(i = 0; two_pos[x].num[i] != 0; i++)
						  integers[two_pos[x].num[i]].dag = -1;
					}
				}
				if(two_neg[x].num[0] > 0) {
					out = 0;
					for(z = 0; (greater_pos[x].num[z] != 0) && (out != 1); z++) {
						count = 0;
						for(y = 0; integers[greater_pos[x].num[z]].num[y] != 0; y++) {
							if(integers[greater_pos[x].num[z]].dag == -1) {
								for(i = 0; two_neg[x].num[i] != 0; i++) {
									if(integers[two_neg[x].num[i]].dag == 2)
									  continue;
									if(((integers[greater_pos[x].num[z]].num[y] == -integers[two_neg[x].num[i]].num[0])
									 &&(-integers[two_neg[x].num[i]].num[0] != x))
									 ||((integers[greater_pos[x].num[z]].num[y] == -integers[two_neg[x].num[i]].num[1])
									 &&(-integers[two_neg[x].num[i]].num[1] != x)))
									  {
										  integers[two_neg[x].num[i]].dag = 2;
										  count++;
									  }
								}
							}
						}
						if(count == y - 1) {
							for(i = 0; two_neg[x].num[i] != 0; i++) {
								if(integers[two_neg[x].num[i]].dag == 2)
								  integers[two_neg[x].num[i]].length = 0;
							}
							and_or_do = 1;
							integers[greater_pos[x].num[z]].dag = 0;
//							for(i = 0; integers[greater_pos[x].num[z]].num[i] != x; i++) {}
//							integers[greater_pos[x].num[z]].andor = i;
							for(i = 0; integers[greater_pos[x].num[z]].num[i] != 0; i++) {}
							integers[greater_pos[x].num[z]].length = i;
							integers[greater_pos[x].num[z]].andor = x;
							out = 1;
							num++;
						}
						for(i = 0; two_neg[x].num[i] != 0; i++)
						  integers[two_neg[x].num[i]].dag = -1;
					}
				}
			}
		}
      
		//Not needed anymore, free them!
		for(int x = 1; x < numinp + 1; x++) {
			free(two_pos[x].num);
			free(two_neg[x].num);
			free(greater_pos[x].num);
			free(greater_neg[x].num);
		}
      free(two_pos);
      free(two_neg);
      free(greater_pos);
      free(greater_neg);
      
		//recurse is needed to seperate the and= and or= from the unclustered
		//clauses. recurse will hold the and= and or= clauses. integers will hold
		//the rest of the unclustered clauses.
		store *recurse;
      recurse = (store *)calloc(num+1, sizeof(store));
      y = -1;
      num = -1;
      for(long x = 1; x < numout+1; x++) {
			if(integers[x].dag != -1) {
				num++;
				recurse[num] = integers[x];
			}
			else if(integers[x].length > 0) {
				y++;
				integers[y] = integers[x];
			}
		}
		
      numout = y+1;
      int recurselen = num+1;
      recurse[recurselen].dag = -1;
      d2_printf1("Done Saving");
      int *threepos_temp = (int *)calloc(numinp+1, sizeof(int));
      int *threeneg_temp = (int *)calloc(numinp+1, sizeof(int));
      for(long x = 0; x < numout; x++) {
	for(y = 0; integers[x].num[y] != 0; y++) {}
	integers[x].length = y;
	if(y == 3) {
	  for(y = 0; y < 3; y++) {
	    if(integers[x].num[y] > 0)
	      threepos_temp[integers[x].num[y]]++;
	    else
	      threeneg_temp[-integers[x].num[y]]++;
	  }
	}
      }
      store *three_pos = (store *)calloc(numinp+1, sizeof(store));
      store *three_neg = (store *)calloc(numinp+1, sizeof(store));
      
		//Store appropriate array sizes to help with memory usage
		for(long x = 1; x < numinp+1; x++) {
			three_pos[x].num = (int *)calloc(threepos_temp[x]+1, sizeof(int));
			three_neg[x].num = (int *)calloc(threeneg_temp[x]+1, sizeof(int));
		}
      free(threepos_temp);
      free(threeneg_temp);
      count = 0;
      
		//Store all clauses with 3 variables so they can be clustered
		for(long x = 0; x < numout; x++) {
			if(integers[x].length == 3) {
				count++;
				for(i = 0; i < 3; i++) {
					if(integers[x].num[i] < 0) {
						three_neg[-integers[x].num[i]].num[three_neg[-integers[x].num[i]].length] = x;
						three_neg[-integers[x].num[i]].length++;
					} else {
						three_pos[integers[x].num[i]].num[three_pos[integers[x].num[i]].length] = x;
						three_pos[integers[x].num[i]].length++;
					}
				}
			}
		}
      
		//v3 is in all clauses
		//if v0 is positive, both v0 positive clauses have v2, just sign changed
		//if v0 is negative, both v0 negative clauses have v1, just sign changed
		//the signs of v1 and v2 are the inverse of the sign of v3
		struct ite_3 {
			int pos;
			int neg;
			int v0;
			int v1;
		};
      struct save_4 {
			long vars[4];
		};
      ite_3 *v3_1 = (ite_3 *)calloc(1000, sizeof(ite_3));
      ite_3 *v3_2 = (ite_3 *)calloc(1000, sizeof(ite_3));
      
		//For Future, spend the time to actually calculate those 1000's
		//It wouldn't be toooo tough.
      int v3_1count;
      int v3_2count;
      save_4 *ites = (save_4 *)calloc(count/2+1, sizeof(save_4));
      d2_printf1 ("Finding ITEs");
      num = 0;
      for(int x = 0; x < numinp+1; x++) {
			v3_1count = 0;
			v3_2count = 0;
			for(i = 0; i < three_pos[x].length; i++) {
				//Finding clauses that have 1 negative variable
				//and clauses that have 2 negative variables
				count = 0;
				for(y = 0; y < 3; y++) {
					if(integers[three_pos[x].num[i]].num[y] < 0)
					  count++;
				}
				if(count == 1) {
					v3_1[v3_1count].pos = three_pos[x].num[i];
					v3_1count++;
				} else if(count == 2) {
					v3_2[v3_2count].pos = three_pos[x].num[i];
					v3_2count++;
				}
			}
			
			//Search through the clauses with 1 negative variable
			//  and try to find counterparts
			for(i = 0; i < v3_1count; i++) {
				out = 0;
				for(y = 0; (y < three_neg[x].length) && (!out); y++) {
					v3_1[i].v0 = -1;
					v3_1[i].v1 = -1;
					count = 0;
					for(z = 0; z < 3; z++) {
					  if(integers[v3_1[i].pos].num[z] != x) {
					    for(j = 0; j < 3; j++) {
					      if((integers[v3_1[i].pos].num[z] == integers[three_neg[x].num[y]].num[j])
						 &&(integers[v3_1[i].pos].num[z] > 0))
						{
						  count++;
						  v3_1[i].v0 = z;
						} else if(-integers[v3_1[i].pos].num[z] ==	integers[three_neg[x].num[y]].num[j])
						  //&&(integers[v3_1[i].pos].num[z]<0))
						  {
						    count++;
						    v3_1[i].v1 = z;
						  }
					    }
					  }
					}
					if(count == 2) {
						//The counterpart clause to v3_1[i].pos is v3_1[i].neg
						v3_1[i].neg = three_neg[x].num[y];
						out = 1;
					}
				}
				if(out == 0) {
				  v3_1[i].v0 = -1;
				  v3_1[i].v1 = -1;
				}
			}
			//Search through the clauses with 2 negative variables
			for(i = 0; i < v3_2count; i++) {
				out = 0;
				for(y = 0; (y < three_neg[x].length) && (!out); y++) {
					v3_2[i].v0 = -1;
					v3_2[i].v1 = -1;
					count = 0;
					for(z = 0; z < 3; z++) {
					  if(integers[v3_2[i].pos].num[z] != x) {
					    for(j = 0; j < 3; j++) {
					      if((integers[v3_2[i].pos].num[z] == integers[three_neg[x].num[y]].num[j])
						 &&(integers[v3_2[i].pos].num[z] < 0))
						{
						  count++;
						  v3_2[i].v0 = z;
						} else if(-integers[v3_2[i].pos].num[z] == integers[three_neg[x].num[y]].num[j])
						  //&&(-integers[v3_2[i].pos].num[z]>0));
						  {
						    count++;
						    v3_2[i].v1 = z;
						  }
					    }
					  }
					}
					if(count == 2) {
					  //The counterpart clause to v3_2[i].pos is v3_2[i].neg
					  v3_2[i].neg = three_neg[x].num[y];
					  out = 1;
					}
				}
				if(out == 0) {
				  v3_2[i].v0 = -1;
				  v3_2[i].v1 = -1;
				}
			}
			out = 0;
			for(i = 0; (i < v3_1count) && (!out); i++) {
				for(y = 0; (y < v3_2count) && (!out); y++) {
					if((v3_1[i].v0 == -1) || (v3_1[i].v1 == -1) 
					 ||(v3_2[y].v0 == -1) || (v3_2[y].v1 == -1))
					  continue;
					if(integers[v3_1[i].pos].num[v3_1[i].v0] == -integers[v3_2[y].pos].num[v3_2[y].v0]) {
						integers[v3_1[i].pos].length = -1;
						integers[v3_1[i].neg].length = -1;
						integers[v3_2[y].pos].length = -1;
						integers[v3_2[y].neg].length = -1;
						ites[num].vars[0] =  integers[v3_1[i].pos].num[v3_1[i].v0];
						ites[num].vars[1] = -integers[v3_2[y].pos].num[v3_2[y].v1];
						ites[num].vars[2] = -integers[v3_1[i].pos].num[v3_1[i].v1];
						ites[num].vars[3] = x;
						for(z = 0; z < three_pos[x].length; z++) {
							count = 0;
							for(j = 0; j < 3; j++) {
								if((-integers[three_pos[x].num[z]].num[j] == ites[num].vars[1])
								 ||(-integers[three_pos[x].num[z]].num[j] == ites[num].vars[2]))
								  count++;
							}
							if(count == 2)
							  integers[three_pos[x].num[z]].length = -1;
						}
						for(z = 0; z < three_neg[x].length; z++)	{
							count = 0;
							for(j = 0; j < 3; j++) {
								if((integers[three_neg[x].num[z]].num[j] == ites[num].vars[1])
								 ||(integers[three_neg[x].num[z]].num[j] == ites[num].vars[2]))
								  count++;
							}
							if(count == 2) integers[three_neg[x].num[z]].length = -1;
						}
						num++;
						out = 1;
					}
				}
			}
		}
      for(int x = 1; x < numinp+1; x++) {
	free(three_pos[x].num);
	free(three_neg[x].num);
      }
      free(three_pos);
      free(three_neg);
      free(v3_1);
      free(v3_2);
      int num_ite = num;
      num = -1;

      for (long x = 0; x < numout+1; x++) {
	if(integers[x].length != -1) {
	  num++;
	  integers[num] = integers[x];
	}
      }
      numout = num;
      d2_printf2("Building ITE BDDs - %d\n", num_ite);
     
      bdd_circuit_init(numinp, numout+num_ite+recurselen+5+num_minmax);

		//Creates BDD's for the ite_equal clauses
      for(long x = 0; x < num_ite; x++) {
	functions[x] = ite_itequ(ite_var(ites[x].vars[0]),
				 ite_var(ites[x].vars[1]),
				 ite_var(ites[x].vars[2]),
				 ite_var(ites[x].vars[3]));
	functionType[x] = ITE;
	independantVars[ites[x].vars[3]] = 0;
      }
      free(ites);
      
      //Idea? Sort the clauses by size ???
      //qsort(integers, numout, sizeof(store), compfunc);
      
      //This or's all the variables in each individual clause
      d2_printf2("Building unclustered BDDs - %ld\n", numout);
      for(long x = 0; x < numout; x++) {
	qsort(integers[x].num, integers[x].length, sizeof(int), abscompfunc);
	//qsort(integers[x].num, integers[x].length, sizeof(int), absrevcompfunc);
	functions[x+num_ite] = false_ptr;
	//	fprintf(stderr, "%d: %d ", x, integers[x].num[0]);
	for(y = 0; y < integers[x].length; y++) {
	  functions[x+num_ite] = ite_or(functions[x+num_ite], ite_var(integers[x].num[y]));
	  //	  fprintf(stderr, "%d ", integers[x].num[y]);
	}
	//	fprintf(stderr, "0\n");
	functionType[x+num_ite] = PLAINOR;
      }
      numout = numout+num_ite;
      
      //Creates BDD's for the and_equal and or_equal clauses
      d2_printf2("Building and= & or= BDDs - %d\n", recurselen);
      for(long x = 0; x < recurselen; x++) {
	qsort(recurse[x].num, recurse[x].length, sizeof(int), abscompfunc);
	//qsort(recurse[x].num, recurse[x].length, sizeof(int), absrevcompfunc);
	if(recurse[x].dag > 0) {
	  functions[x + numout] = false_ptr;
	  for(y = 0; y < recurse[x].length; y++) {
	    if(recurse[x].num[y] != recurse[x].andor)
	      functions[x+numout] = ite_or(functions[x+numout], ite_var(recurse[x].num[y]));
	  }
	  functionType[x+numout] = OR;
	  equalityVble[x + numout] = -recurse[x].andor;
	  independantVars[equalityVble[x+numout]] = 0;
	  functions[x+numout] = ite_equ(ite_var(-recurse[x].andor), functions[x+numout]);
	} else {
	  functions[x+numout] = true_ptr;
	  for(y = 0; y < recurse[x].length; y++) {
	    if(recurse[x].num[y] != recurse[x].andor)
	      functions[x + numout] = ite_and(functions[x+numout], ite_var(-recurse[x].num[y]));
	  }
	  functionType[x+numout] = AND;
	  equalityVble[x+numout] = recurse[x].andor;
	  independantVars[equalityVble[x+numout]] = 0;
	  functions[x+numout] = ite_equ(ite_var(recurse[x].andor), functions[x + numout]);
	}
      }
      free(recurse);
      numout = numout+recurselen;
      d2_printf2("Building MinMax BDDs - %d\n", num_minmax);
      for(int x = 0; x < num_minmax; x++) {
	int set_true = 0;
	qsort(min_max_store[x].num, min_max_store[x].length, sizeof(int), abscompfunc);
	functions[x+numout] = MinMaxBDD(min_max_store[x].num, min_max_store[x].min, min_max_store[x].max, min_max_store[x].length, set_true);
      }
      delete [] min_max_store;
      numout = numout+num_minmax;
	} else {
	  bdd_circuit_init(numinp, numout+num_minmax);
	  
	  d2_printf2("Building unclustered BDDs - %ld\n", numout);
	  
	  for(long x = 1; x < numout + 1; x++) {
	    if (x % 1000 == 0) d2_printf2("%ld\r", x);
	    qsort(integers[x].num, integers[x].length, sizeof(int), abscompfunc);
	    //qsort(integers[x].num, integers[x].length, sizeof(int), absrevcompfunc);
	    functions[x-1] = false_ptr;
	    //fprintf(stderr, "\n%d ", integers[x].num[0]);
	    for(y = 0; y < integers[x].length; y++) {
	      functions[x - 1] = ite_or (functions[x-1], ite_var(integers[x].num[y]));
				//fprintf(stderr, "%d ", integers[x].num[y]);
	    }
	    functionType[x-1] = PLAINOR;	  
	  }
	  d2_printf2("Building MinMax BDDs - %d\n", num_minmax);
	  for(int x = 0; x < num_minmax; x++) {
	    int set_true = 0;
	    qsort(min_max_store[x].num, min_max_store[x].length, sizeof(int), abscompfunc);
	    functions[x+numout] = MinMaxBDD(min_max_store[x].num, min_max_store[x].min,
					    min_max_store[x].max, min_max_store[x].length,
					    set_true);
	  }
	  delete [] min_max_store;
	  numout = numout+num_minmax;
	}
	count = -1;
	
	d2_printf1 ("Cleaning Up");
	
	//Need to remove any clause that was set to True during the making of the BDDs
	for(long x = 0; x < numout; x++) {
      count++;
      functions[count] = functions[x];
      equalityVble[count] = equalityVble[x];
      functionType[count] = functionType[x];
      if (functions[x] == true_ptr)
		  count--;
	}
	
	for(long x = 1; x < old_numout + 1; x++) {
		delete [] old_integers[x].num;
	}
	delete [] integers;
	delete [] old_integers;
	
	numout = count+1;
	nmbrFunctions = numout;
	d2_printf2("Number of BDDs - %ld\n", numout);
}

void DNF_to_CNF () {
	typedef struct {
		int num[50];
   } node1;
	node1 *integers;
	char string;
	int lines = 0, length;
	int y = 0;
	fscanf(finputfile, "%ld %ld\n", &numinp, &numout);
	length = numout;
	integers = new node1[numout+1];
	for(int x = 0; x < length; x++) {
      y = 0;
      
      do {
			fscanf(finputfile, "%d", &integers[x].num[y]);
			string = fgetc(finputfile);
			y++;
			lines++;
		}
      while(integers[x].num[y - 1] != 0);
	}
	char string1[1024];
	lines = lines + 1;
	sprintf(string1, "p cnf %ld %d\n", numinp + length, lines);
	fprintf(foutputfile, "%s", string1);
	for(int y = 0; y < length; y++) {
      sprintf(string1, "%ld ", y + numinp + 1);
      fprintf(foutputfile, "%s", string1);
	}
	sprintf(string1, "0\n");
	fprintf(foutputfile, "%s", string1);
	for(int x = 0; x < numout; x++) {
      sprintf(string1, "%ld ", x + numinp + 1);
      fprintf(foutputfile, "%s", string1);
      for(int y = 0; integers[x].num[y] != 0; y++) {
			sprintf(string1, "%d ", -integers[x].num[y]);
			fprintf(foutputfile, "%s", string1);
		}
      sprintf(string1, "0\n");
      fprintf(foutputfile, "%s", string1);
      for(int y = 0; integers[x].num[y] != 0; y++) {
			sprintf(string1, "%ld ", -(x + numinp + 1));
			fprintf(foutputfile, "%s", string1);
			sprintf(string1, "%d ", integers[x].num[y]);
			fprintf(foutputfile, "%s", string1);
			sprintf(string1, "0\n");
			fprintf(foutputfile, "%s", string1);
		}
	}
	sprintf(string1, "c\n");
	fprintf(foutputfile, "%s", string1);
}
