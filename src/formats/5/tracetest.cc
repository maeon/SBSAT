#include "ite.h"
#include "bddnode.h"

int *length;
BDDNode **xorFunctions;
char filename[128]="stdin";
extern int s_error;

//#define IN trace_in
//#define PARSE trace_parse

#define IN blif_in
#define PARSE blif_parse

extern FILE *IN;
void PARSE();

int main(int argc, char **argv)
{
    parser_init();

    ++argv, --argc;  /* skip over program name */
    if ( argc > 0 ) {
	while (s_error==0 && argc > 0) {
	    strncpy(filename, argv[0], 127);
	    printf("Reading: %s\n", filename); fflush(stdout);
	    if ( (IN = fopen( filename, "r" )) != NULL) {
	       printf(".");
	       PARSE();
	       fclose(IN);
	    }
    	    ++argv, --argc;  
	}
    }
    else
    {
            IN = stdin;
    	    PARSE();
    }
    //bddnode_free();
    return 0;
}

