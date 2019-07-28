/* main.c (dict_hashes) -- (c)2019 Henrique Moreira

   Checks, runs, hashes dictionaries.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_PROG_NAME "hashing_div"

#include "hashing_div_lang.h"


void usage (const char* prog)
{
 printf("%s command [options ...] [args]\n\
\n\
Where commands are:\n\
   dump [file ...]\n\
\n\
Options are:\n\
   ToDo\n\
", prog);
 exit(0);
}


/* Forward declarations
 */
int hash_dump (FILE* fOut, t_bool isStdin, char** args) ;



/* Main run
 */
int run (const char* prog, const char* strCmd, int nArgs, char** args)
{
 int code = -1;
 FILE* fOut = stdout;
 t_bool isStdin;
 char** pFiles;
 const t_bool isMainProg = strcmp( prog, BASE_PROG_NAME )==0;

 bprint("prog=<%s>, strCmd=<%s>, nArgs=%d, 1st:<%s>\n",
	prog, strCmd, nArgs, args[ 0 ]);
 b_assert(isMainProg,"?");
 b_assert(args,"args");

 pFiles = args+1;
 isStdin = pFiles[ 0 ]==NULL;
 if ( isStdin ) {
     pFiles = (char**)malloc( 2*sizeof(char*) );
     pFiles[ 0 ] = strdup("-");
     pFiles[ 1 ] = NULL;
 }
 if ( strcmp( strCmd, "dump" )==0 ) {
     code = hash_dump( fOut, isStdin, pFiles );
 }
 if ( isStdin ) {
     free( pFiles[ 0 ] );
     free( pFiles );
 }
 if ( code==-1 ) {
     usage( prog );
 }
 return code;
}


int hash_dump (FILE* fOut, t_bool isStdin, char** pFiles)
{
 FILE* fIn = stdin;
 FILE* fErr = stderr;
 const char* name;
 char buf[ 1024 ];
 int i = 0;
 int invalid = 0;
 int nBogus = 0;
 int aLen;
 char c;
 t_uint32 h;

 for ( ; (name = pFiles[i])!=NULL; i++) {
     int lines = 0;
     bprint("Reading: %s\n", name);
     if ( !isStdin ) {
	 fIn = fopen( name, "rb" );
     }
     if ( fIn==NULL ) {
	 fprintf(fErr, "Uops: %s\n", name);
	 return 2;
     }
     for ( ; fgets( buf, sizeof(buf)-1, fIn ); ) {
	 lines++;
	 aLen = strlen( buf );
	 b_assert(aLen>0,"aLen>0");
	 aLen--;
	 c = buf[ aLen ];
	 if ( c!='\n' ) {
	     invalid++;
	     break;
	 }
	 buf[ aLen ] = 0;
	 h = hd_str_hash( buf );
	 fprintf(fOut, "%-10ld\t%s\n", h, buf);
     }
     if ( !isStdin ) {
	 fclose( fIn );
     }
     if ( invalid ) {
	 nBogus++;
	 fprintf(fErr,"Invalid: %s\n", name);
     }
     else {
	 fprintf(fErr, "Read %s: %d line(s)\n", name, lines);
     }
 }
 return nBogus>0;
}


/* Main program
 */
int main (int argc, char* argv[])
{
 int code;
 const char* prog;
 const char* strCmd;
 /*
   How To compile:
	gcc -Werror -Wall -o main.o -c main.c
	gcc *.o -o hashing_div
 */
 prog = strrchr( argv[ 0 ], '/' );
 if ( prog==NULL ) {
     prog = strrchr( argv[ 0 ], '\\' );
 }
 if ( prog ) {
     prog++;
 }
 else {
     prog = argv[ 0 ];
 }
 if ( argc<2 ) {
     usage( prog );
 }
 strCmd = argv[ 1 ];
 code = run( prog, strCmd, argc-1, argv+1 );
 return code;
}
