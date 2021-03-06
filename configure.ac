#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([sbsat], [2.7b], [weaversa@gmail.com])
AC_CONFIG_SRCDIR([doc/ftest.c])
AC_CONFIG_HEADERS([config.h])
# AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([tar-pax])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LEX
AC_PROG_YACC

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([float.h inttypes.h libintl.h limits.h malloc.h memory.h stddef.h stdint.h stdlib.h string.h strings.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRCOLL
AC_FUNC_WAIT3
AC_CHECK_FUNCS([bzero dup2 gethostname getpagesize gettimeofday memchr memmove memset pow regcomp select sqrt strcasecmp strchr strcspn strdup strerror strncasecmp strpbrk strrchr strspn strstr strtoul])

# Check for executables.
AC_CHECK_PROG(HELP2MAN,help2man,yes)
AC_SUBST(HELP2MAN)
if test x"$HELP2MAN" != x"yes" ; then
    AC_MSG_WARN([help2man not found -- will not update man page.])
fi

# Other things
AM_CONDITIONAL(SBSAT_STATIC, test x$sbsat_static = xtrue)

cvsbuild=false
AC_ARG_ENABLE(cvsbuild,
	      [  --enable-cvsbuild Create nightly build],
	      [case "${enableval}" in
	         no) cvsbuild=false ;;
		   *) cvsbuild=${enableval} ;;
	   esac],[debug=false])
AM_CONDITIONAL(NO_CVSBUILD, test x$cvsbuild = xfalse)
CVSBUILD=$cvsbuild
AC_SUBST(CVSBUILD)

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 doc/man1/Makefile
                 doc/manual/Makefile
                 doc/manual/sbsat_user_manual/Makefile
                 examples/Makefile
                 examples/example_tests.sh
                 include/Makefile
                 src/Makefile
                 src/cudd/Makefile
                 src/cudd/cppobj/Makefile
                 src/cudd/cudd/Makefile
                 src/cudd/epd/Makefile
                 src/cudd/include/Makefile
                 src/cudd/mtr/Makefile
                 src/cudd/st/Makefile
                 src/cudd/util/Makefile
                 src/formats/5/Makefile
                 src/formats/Makefile
                 src/generator/Makefile
                 src/postproc/Makefile
                 src/preproc/Makefile
                 src/solv_bddwalk/Makefile
                 src/solv_dac/Makefile
                 src/solv_simple/Makefile
                 src/solv_simple/bb_gelim/Makefile
                 src/solv_simple/bb_lemmas/Makefile
                 src/solv_simple/fn_inference/Makefile
                 src/solv_simple/fn_minmax/Makefile
                 src/solv_simple/fn_negminmax/Makefile
                 src/solv_simple/fn_or/Makefile
                 src/solv_simple/fn_smurf/Makefile
                 src/solv_simple/fn_xor/Makefile
                 src/solv_simple/include/Makefile
                 src/solv_smurf/Makefile
                 src/solv_smurf/fn_and/Makefile
                 src/solv_smurf/fn_lemma/Makefile
                 src/solv_smurf/fn_lemma_au/Makefile
                 src/solv_smurf/fn_minmax/Makefile
                 src/solv_smurf/fn_smt_bitvector/Makefile
                 src/solv_smurf/fn_smurf/Makefile
                 src/solv_smurf/fn_smurf_au/Makefile
                 src/solv_smurf/fn_smurf_xor/Makefile
                 src/solv_smurf/fn_xor/Makefile
                 src/solv_smurf/fn_xor_smurf/Makefile
                 src/solv_smurf/hr_anne/Makefile
                 src/solv_smurf/hr_berkmin/Makefile
                 src/solv_smurf/hr_lemma/Makefile
                 src/solv_smurf/hr_lsgb/Makefile
                 src/solv_smurf/hr_lsgb_lemma/Makefile
                 src/solv_smurf/hr_lsgbw/Makefile
                 src/solv_smurf/hr_minisat/Makefile
                 src/solv_smurf/hr_null/Makefile
                 src/solv_smurf/hr_vsids/Makefile
                 src/solv_smurf/include/Makefile
                 src/solv_wvf/Makefile
                 src/solver/Makefile
                 src/utils/Makefile
                 tests/Makefile
                 tests/trace_tests.sh
                 tests/longer_tests.sh
                 tests/cnf_tests.sh
                 tests/xor_tests.sh
                 tests/cnf/Makefile
                 tests/longer_tests/Makefile
                 tests/trace_tests/Makefile])
AC_OUTPUT
