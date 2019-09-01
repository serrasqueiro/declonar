/* main.c (dict_hashes) -- (c)2019 Henrique Moreira

   Checks, runs, hashes dictionaries.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_PROG_NAME "hashing_div"

#include "hashing_div_lang.h"
#include "nospell.h"


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



/* Globals
 */
static t_word_dict gDict;


/* Basic aux. functions
 */
void init_opt_hshow (t_opt_hshow* st)
{
 b_assert(st, "?!");
 st->verbose = 0;
 st->minSize = 1;
 st->lines = 0;
 st->strFilePath = NULL;
 st->strOutPat = strdup( "letra_@.asc" );
 st->zero = 0;
 st->hashgram = NULL;
}


/* Forward declarations
 */
int hash_dump (FILE* fOut, t_bool isStdin, char** args, t_opt_hshow* ptrShow) ;
int hash_hist (FILE* fOut, FILE* fIn, unsigned binSize, t_opt_hshow* ptrShow) ;
int to_base (FILE* fOut, t_bool isStdin, char** pFiles, t_opt_hshow* ptrShow) ;



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

 init_dict( &gDict );

 args++;

 for ( ; args[ 0 ]!=NULL; ) {
     bprint("Debug: checking args#0: '%s'\n", args[0]);
     if ( strcmp( args[ 0 ], "-v" )==0 ) {
	 verbose++;
	 optShow.verbose++;
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
     if ( strcmp( args[ 0 ], "-a" )==0 ) {
	 b_assert(args[ 1 ], "-a?");
	 free( optShow.strOutPat );
	 optShow.strOutPat = strdup( args[ 1 ] );
	 if ( strchr(optShow.strOutPat,'@')==NULL ) {
	     usage( prog );
	 }
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
 if ( strcmp( strCmd, "to-base" )==0 ) {
     code = to_base( fOut, isStdin, pFiles, &optShow );
     fprintf(fErr, "Code: %d\n", code);
 }

 if ( strcmp( strCmd, "hist" )==0 ) {
     const int verBinSize = binSize;
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
	 for (z=verBinSize-1; z>=lowEnd; z--) {
	     for (h=0; h<verBinSize; h++) {
		 int num =  optShow.hashgram[ h ].count;
		 if ( num==z ) {
		     fprintf(fErr, "# %d %s\n", num, optShow.hashgram[ h ].str);
		 }
	     }
	 }
     }
     else {
	 for (h=0; h<verBinSize; h++) {
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
 /* Free allocated option vars */
 free( optShow.strOutPat );

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


int to_base (FILE* fOut, t_bool isStdin, char** pFiles, t_opt_hshow* ptrShow)
{
 FILE* fIn = stdin;
 FILE* fErr = stderr;
 FILE* fAsc = NULL;
 const char* name;
 const int verbose = ptrShow->verbose;
 const char* outputPattern = ptrShow->strOutPat;
 t_uchar c, up, first, last='\0';
 char ch;
 char buf[ 1024 ];
 int i = 0, idx, aLen;
 int letra;
 int sz;
 t_word* w;
 t_bool isOk;
 t_bool allUp;
 t_set* pSet;

 for ( ; (name = pFiles[i])!=NULL; i++) {
     if ( verbose>0 ) {
	 printf("Reading: %s\n", name);
     }
     if ( !isStdin ) {
	 fIn = fopen( name, "rb" );
     }
     if ( fIn==NULL ) {
	 fprintf(fErr, "Uops: %s\n", name);
	 return 2;
     }
     for ( ; fgets( buf, sizeof(buf)-1, fIn ); ) {
       c = buf[ aLen = strlen( buf )-1 ];
       b_assert(c=='\n', "No NL?");
       buf[ aLen ] = 0;
       if ( verbose>=3 ) {
	 printf("%s\n", buf);
       }
       allUp = tTrue;
       for (idx=0, first='\0'; idx<aLen; idx++) {
	 c = buf[ idx ];
	 if ( c>='a' && c<='z' ) {
	   up = c-32;
	   allUp = tFalse;
	 }
	 else {
	   up = c;
	 }
	 if ( idx==0 ) {
	   first = up;
	 }
	 isOk = up>='A' && up<='Z';
	 if ( isOk==tFalse ) {
	   b_assert(c>' ', "(1) No blanks!");
	   b_assert(c!='-' || allUp || (c=='-' && last>='a' && last<='z'),"(2) Dash (-)");
	 }
	 last = c;
       }
       b_assert(first>='A' && first<='Z', "(3) First chr.");
       idx = (int)first - (int)'A';
       w = add_dict( &gDict, idx, (t_uchar*)buf );
       if ( w==NULL ) {
	 if ( verbose>0 ) {
	     fprintf(stderr, "Ignored duplicate word: '%s'\n", buf);
	 }
       }
     }
     if ( !isStdin ) {
	 fclose( fIn );
     }
 }

 if ( verbose>0 ) {
     for (letra=0; letra<26; letra++) {
	 pSet = &gDict.rows[ letra ];
	 printf("Letter %c (%d):\t", letra+'A', (int)pSet->nElems);
	 for (w=pSet->start; w; w=w->next) {
	     printf("%s;", w->s);
	     b_assert(w->next || (w->next==NULL && pSet->end==w), "end?");
	 }
	 printf("\n");
     }
 }

 if ( outputPattern ) {
     char* s;
     char* outStr = strdup( outputPattern );
     b_assert(outStr, "outStr");
     s = strchr( outStr, '@' );
     if ( s ) {
	 for (letra=0; letra<26; letra++) {
	     ch = letra+'A';
	     s[ 0 ] = ch;
	     if ( verbose>0 ) {
		 printf("%c: S is '%s'\n", ch, outStr);
	     }
	     fAsc = fopen( outStr, "w" );
	     if ( fAsc ) {
		 pSet = &gDict.rows[ letra ];
		 for (sz=1; sz<=WORD_DICT_LIM; sz++) {
		     for (w=pSet->bySize[ sz ]; w; w=w->next) {
			 fprintf(fAsc, "%c %s\n", w->kind, w->s);
		     }
		 }
		 fclose( fAsc );
	     }
	 }
	 s[ 0 ] = '1';
	 fAsc = fopen( outStr, "w" );
	 for (letra=0; letra<26; letra++) {
	     char* newStr = (char*)malloc( WORD_DICT_LIM * 10 );
	     newStr[ 0 ] = 0;
	     for (sz=1; sz<=WORD_DICT_LIM; sz++) {
		 char b[ 64 ];
		 int elems = 0;
		 t_word* iter = iter=gDict.rows[ letra ].bySize[ sz ];
		 for ( ; iter; iter=iter->next) {
		     elems++;
		 }
		 sprintf(b, " %d;", elems);
		 strcat(newStr, b);
	     }
	     fprintf(fAsc, "letter %c: %lu %s\n", letra+'A', (unsigned long)gDict.rows[ letra ].nElems, newStr);
	     free( newStr );
	 }
	 fclose( fAsc );
     }
     free( outStr );
 }
 return 0;
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
 for (h=0; h<binSize; h++) {
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
 release_dict( &gDict );
 /* Exit status */
 return code;
}
