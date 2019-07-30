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


/* Basic aux. functions
 */
void init_opt_hshow (t_opt_hshow* st)
{
 b_assert(st, "?!");
 st->verbose = 0;
 st->minSize = 1;
 st->lines = 0;
 st->strFilePath = NULL;
 st->zero = 0;
 st->hashgram = NULL;
}


/* Forward declarations
 */
int hash_dump (FILE* fOut, t_bool isStdin, char** args, t_opt_hshow* ptrShow) ;
int hash_hist (FILE* fOut, FILE* fIn, unsigned binSize, t_opt_hshow* ptrShow) ;




/* Main run
 */
int run (const char* prog, const char* strCmd, int nArgs, char** args)
{
 int code = -1;
 int verbose = 0;
 FILE* fIn = stdin;
 FILE* fOut = stdout;
 FILE* fErr = stderr;
 t_bool isStdin;
 char** pFiles;
 const char* inName = NULL;
 const t_bool isMainProg = strcmp( prog, BASE_PROG_NAME )==0;
 unsigned binSize = 1000;
 t_opt_hshow optShow;

 init_opt_hshow( &optShow );
 optShow.verbose = verbose;

 bprint("prog=<%s>, strCmd=<%s>, nArgs=%d, 1st:<%s>\n",
	prog, strCmd, nArgs, args[ 0 ]);
 b_assert(isMainProg,"?");

 args++;

 for ( ; args[ 0 ]!=NULL; ) {
     bprint("Debug: checking args#0: '%s'\n", args[0]);
     if ( strcmp( args[ 0 ], "-v" )==0 ) {
	 verbose++;
	 args++;
	 continue;
     }
     if ( strcmp( args[ 0 ], "-n" )==0 ) {
	 b_assert(args[ 1 ], "-n?");
	 optShow.minSize = atoi( args[ 1 ] );
	 b_assert(optShow.minSize>0 && optShow.minSize<9, "-n invalid");
	 args += 2;
	 continue;
     }
     if ( args[ 0 ][ 0 ]=='-' ) {
	 usage( prog );
     }
     break;
 }

 pFiles = args;
 isStdin = pFiles[ 0 ]==NULL;
 if ( isStdin ) {
     pFiles = (char**)malloc( 2*sizeof(char*) );
     pFiles[ 0 ] = strdup("-");
     pFiles[ 1 ] = NULL;
 }
 if ( strcmp( strCmd, "dump" )==0 ) {
     optShow.verbose = 1;
     code = hash_dump( fOut, isStdin, pFiles, &optShow );
     if ( code==0 ) {
	 fprintf(fErr, "Last: %s, %d line(s)\n", optShow.strFilePath, optShow.lines);
     }
 }
 if ( strcmp( strCmd, "hist" )==0 ) {
     const int primeSize = hist_prime_size( binSize );
     const int lowEnd = 4;
     t_bool resumed = verbose==0;
     t_uint32 h;
     int z;
     inName = pFiles[ 0 ];
     if ( !isStdin ) {
	 fIn = fopen( inName, "rt" );
     }
     code = hash_hist( fOut, fIn, binSize, &optShow );
     if ( !isStdin ) {
	 fclose( fIn );
     }
     if ( resumed ) {
	 for (z=primeSize-1; z>=lowEnd; z--) {
	     for (h=0; h<primeSize; h++) {
		 int num =  optShow.hashgram[ h ].count;
		 if ( num==z ) {
		     fprintf(fErr, "# %d %s\n", num, optShow.hashgram[ h ].str);
		 }
	     }
	 }
     }
     else {
	 for (h=0; h<primeSize; h++) {
	     int num =  optShow.hashgram[ h ].count;
	     if ( num > 3 ) {
		 fprintf(fErr, "# %d %s\n", num, optShow.hashgram[ h ].str);
	     }
	 }
     }
     for (h=0; h<binSize; h++) {
	 if ( optShow.hashgram[ h ].str ) {
	     free( optShow.hashgram[ h ].str );
	 }
     }
     fprintf(fErr, "Empties: %d (lines: %d)\n", optShow.zero, optShow.lines);
     free( optShow.hashgram );
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


int hash_dump (FILE* fOut, t_bool isStdin, char** pFiles, t_opt_hshow* ptrShow)
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
     ptrShow->strFilePath = name;
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
	 b_assert(aLen>0, "aLen>0");
	 aLen--;
	 c = buf[ aLen ];
	 if ( c!='\n' ) {
	     invalid++;
	     break;
	 }
	 buf[ aLen ] = 0;
	 h = lang_str_hash( buf );
	 if ( aLen<=0 || buf[ 0 ]=='#' ) {
	     if ( ptrShow->verbose>0 ) {
		 fprintf(fErr, "Ignored entry: '%s'\n", buf);
	     }
	     continue;
	 }
	 if ( aLen < ptrShow->minSize ) {
	     invalid++;
	     fprintf(fErr, "Entry too short: '%s'\n", buf);
	     break;
	 }
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
	 ptrShow->lines += lines;
     }
 }
 return nBogus>0;
}


int hash_hist (FILE* fOut, FILE* fIn, unsigned binSize, t_opt_hshow* ptrShow)
{
 const t_bool showHash = tTrue;
 char buf[ 1024 ];
 int aLen;
 int n = ptrShow->minSize;
 t_opt_hist* lista;
 const char* info;
 const char* fmtDigStr = "03";
 const char* shown;
 t_uint32 h;
 t_uint32 empties = 0;

 b_assert(n==4, "n==4");

 lista = (t_opt_hist*)calloc( binSize, sizeof(t_opt_hist) );
 b_assert(lista, "lista");
 ptrShow->hashgram = lista;

 for ( ; fgets( buf, sizeof(buf)-1, fIn ); ) {
     aLen = strlen( buf );
     b_assert(aLen>0, "aLen>0");
     aLen--;
     buf[ aLen ] = 0;
     h = lang_str_hash( buf );
     b_assert(h<binSize, "h<binSize");
     ptrShow->lines++;
     opt_hist_add( &lista[ h ], buf );
 }
 if ( showHash ) {
     snprintf(buf, sizeof(buf), "%%%sld\t#%%d %%s\n", fmtDigStr);
 }
 else {
     snprintf(buf, sizeof(buf), "%%%sld\t%%s\n", fmtDigStr);
 }
 for (h=0; h<hist_prime_size( binSize ); h++) {
     info = lista[ h ].str;
     shown = info ? info : "-";
     if ( fOut ) {
	 if ( showHash ) {
	     fprintf(fOut, buf, h, ptrShow->hashgram[ h ].count, shown);
	 }
	 else {
	     fprintf(fOut, buf, h, shown);
	 }
     }
     if ( info==NULL ) {
	 empties++;
     }
 }
 ptrShow->zero = empties;
 return 0;
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
