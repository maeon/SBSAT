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
/*
 * logging support facility
 *
 */

/*
 * What level of debugging gets compiled in.
 * Change this to a lower number for better performance
 */
#define _DEBUG_LVL_COMPILE 9

extern FILE* stddbg;
extern int DEBUG_LVL;

/* -- finish this -- *
#ifdef GCC
#define ITE_DEBUG(fmt, args...)  ITE_ERR("ite(%s): "fmt, __FUNCTION__, ## args)
#endif
 * ----------------- */

#define D_E(x) { x }
#define D_0(x) { x }

#if _DEBUG_LVL_COMPILE >= 1
# define D_1(x) { if (DEBUG_LVL >= 1 && DEBUG_LVL<10) { x; } }
#else
# define D_1(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 2
# define D_2(x) if (DEBUG_LVL >= 2 && DEBUG_LVL<10) { x }
#else
# define D_2(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 3
# define D_3(x) if (DEBUG_LVL >= 3 && DEBUG_LVL<10) { x }
#else
# define D_3(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 4
# define D_4(x) if (DEBUG_LVL >= 4 && DEBUG_LVL<10) { x }
#else
# define D_4(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 5
# define D_5(x) if (DEBUG_LVL >= 5 && DEBUG_LVL<10) { x }
#else
# define D_5(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 6
# define D_6(x) if (DEBUG_LVL >= 6 && DEBUG_LVL<10) { x }
#else
# define D_6(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 8
# define D_8(x) if (DEBUG_LVL >= 8 && DEBUG_LVL<10) { x }
#else
# define D_8(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 9
# define D_9(x) if (DEBUG_LVL >= 9 && DEBUG_LVL<10) { x }
# define TB_9(x) if (DEBUG_LVL >= 9 && nNumBacktracks >= TRACE_START) { x } 
#else
# define D_9(x) {}
# define TB_9(x) {}
#endif

#define dE_printf1(x)        D_E(fprintf(stddbg, x);)
#define dE_printf2(x1,x2)    D_E(fprintf(stddbg, x1, x2);)
#define dE_printf3(x1,x2,x3) D_E(fprintf(stddbg, x1, x2, x3);)
#define dE_printf4(x1,x2,x3,x4) D_E(fprintf(stddbg, x1, x2, x3,x4);)

#define d0_printf1(x)        D_0(fprintf(stddbg, x);)
#define d0_printf2(x1,x2)    D_0(fprintf(stddbg, x1, x2);)
#define d0_printf3(x1,x2,x3) D_0(fprintf(stddbg, x1, x2, x3);)
#define d0_printf4(x1,x2,x3,x4) D_0(fprintf(stddbg, x1, x2, x3,x4);)

#define d1_printf1(x)        D_1(fprintf(stddbg, x);)
#define d1_printf2(x1,x2)    D_1(fprintf(stddbg, x1, x2);)
#define d1_printf3(x1,x2,x3) D_1(fprintf(stddbg, x1, x2, x3);)
#define d1_printf4(x1,x2,x3,x4) D_1(fprintf(stddbg, x1, x2, x3,x4);)

#define d2_printf1(x)        D_2(fprintf(stddbg, x);)
#define d2_printf2(x1,x2)    D_2(fprintf(stddbg, x1, x2);)
#define d2_printf3(x1,x2,x3) D_2(fprintf(stddbg, x1, x2, x3);)
#define d2_printf4(x1,x2,x3,x4) D_2(fprintf(stddbg, x1, x2, x3,x4);)
#define d2_printf5(x1,x2,x3,x4,x5) D_2(fprintf(stddbg, x1, x2, x3,x4,x5);)

#define d3_printf1(x)        D_3(fprintf(stddbg, x);)
#define d3_printf2(x1,x2)    D_3(fprintf(stddbg, x1, x2);)
#define d3_printf3(x1,x2,x3) D_3(fprintf(stddbg, x1, x2, x3);)
#define d3_printf4(x1,x2,x3,x4) D_3(fprintf(stddbg, x1, x2, x3,x4);)

#define d4_printf1(x)        D_4(fprintf(stddbg, x);)
#define d4_printf2(x1,x2)    D_4(fprintf(stddbg, x1, x2);)
#define d4_printf3(x1,x2,x3) D_4(fprintf(stddbg, x1, x2, x3);)
#define d4_printf4(x1,x2,x3,x4) D_4(fprintf(stddbg, x1, x2, x3,x4);)

#define d5_printf1(x)        D_5(fprintf(stddbg, x);)
#define d5_printf2(x1,x2)    D_5(fprintf(stddbg, x1, x2);)
#define d5_printf3(x1,x2,x3) D_5(fprintf(stddbg, x1, x2, x3);)
#define d5_printf4(x1,x2,x3,x4) D_5(fprintf(stddbg, x1, x2, x3,x4);)

#define d6_printf1(x)        D_6(fprintf(stddbg, x);)
#define d6_printf2(x1,x2)    D_6(fprintf(stddbg, x1, x2);)
#define d6_printf3(x1,x2,x3) D_6(fprintf(stddbg, x1, x2, x3);)
#define d6_printf4(x1,x2,x3,x4) D_6(fprintf(stddbg, x1, x2, x3,x4);)

#define d8_printf1(x)        D_8(fprintf(stddbg, x);)
#define d8_printf2(x1,x2)    D_8(fprintf(stddbg, x1, x2);)
#define d8_printf3(x1,x2,x3) D_8(fprintf(stddbg, x1, x2, x3);)
#define d8_printf4(x1,x2,x3,x4) D_8(fprintf(stddbg, x1, x2, x3,x4);)
#define d8_printf5(x1,x2,x3,x4,x5) D_8(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d8_printf6(x1,x2,x3,x4,x5,x6) D_8(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)

#define d9_printf1(x)        D_9(fprintf(stddbg, x);)
#define d9_printf2(x1,x2)    D_9(fprintf(stddbg, x1, x2);)
#define d9_printf3(x1,x2,x3) D_9(fprintf(stddbg, x1, x2, x3);)
#define d9_printf4(x1,x2,x3,x4) D_9(fprintf(stddbg, x1, x2, x3,x4);)
#define d9_printf5(x1,x2,x3,x4,x5) D_9(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d9_printf6(x1,x2,x3,x4,x5,x6) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)

