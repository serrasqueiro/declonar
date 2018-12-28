/*
  htsix.c -- the quickest HTTP robot

  (c)2018  Henrique Moreira
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>

#include "opts.h"
#include "cbath.h"


static t_debug_level debugLevel=0;
static t_opt_tuple pTuples[]={
   { "", "--", 0, -1, "", NULL },
   { "-V", "--version", 0, -1, "", NULL },
   { "-p", "--port", 9, 80, "80", NULL },
   { "-v", "--verbose", 0, 0, "^", NULL },
   { "-d", "--debug", 9, 0, "0", NULL },
   { "-1", "--HTTP/1.0", 0, 0, "", NULL },
   { "", NULL, 0, 0, NULL, NULL }
};
static t_uchar mainBuffer[ 4096 ];

/* ------------------------------------
   Forward declarations
   ------------------------------------
*/
int do_connect (FILE* fReport, t_colibri* pop, int resolve) ;


/* ------------------------------------
   Parse options
   ------------------------------------
*/
void usage ()
{
 #define main_prog "htsix"
 printf(main_prog " -- A fast HTTP robot\n\
\n\
Usage:\n\
  " main_prog "      head path1 [path ...]\n\
");
 oprint("\n" main_prog " version {%s}, build {%s}\n",
	VERSION,
	BUILD);
}


int parse_options (FILE* fRepErr, int argc, char** argv, t_opts* opts)
{
 int i=0;
 int iter;
 short iRaw, nRaw = 0;
 const int skipMe = INT_MAX_32bit-1;
 const char* str;

 for (i=1; i<argc; i++) {
   int optIdx = -1;
   str = argv[ i ];
   if ( str[ 0 ]=='-' ) {
     int chrIdx;
     t_bool validOpt = False;
     nRaw = ++(opts->numOptions);
     if ( nRaw >= 128 ) {
       return 1;
     }
     opts->rawOpts[ nRaw ] = argv[ i ];
     for (chrIdx=0; chrIdx<strlen( str ); chrIdx++) {
       int iter = 1;
       for ( ; pTuples[iter].shortOpt[0]; iter++) {
         if ( chrIdx==0 ) {
	   if ( strcmp(str, pTuples[iter].shortOpt)==0 || strcmp(str, pTuples[iter].longOpt)==0 ) {
	     optIdx = iter;
	     pTuples[optIdx].valueStr = argv[ i ];
	   }
	   if ( optIdx!=-1 ) {
	     if ( pTuples[optIdx].hasArg ) {
	       i++;
	       //printf("Check arg: {%s}, i=%d, optIdx=%d, next: {%s}\n", str, i, optIdx, argv[ i ]);
	       if ( i>=argc ) return 1;
	       pTuples[optIdx].valueStr = argv[ i ];
	     }
	     else {
	       pTuples[optIdx].iDefault = 1;
	       pTuples[optIdx].strDefault = argv[ i ];
	     }
	     chrIdx = skipMe;
	     break;
	   }
	 }
	 else {
	   char firstChr = pTuples[iter].shortOpt[0];
	   if ( firstChr=='-' && str[chrIdx]==pTuples[iter].shortOpt[1] ) {
	     int defThere = pTuples[optIdx].iDefault;
	     optIdx = iter;
	     pTuples[optIdx].valueStr = (char*)str;
	     if ( defThere>=0 ) {
	       if ( pTuples[optIdx].strDefault[0]=='^' ) {
		 pTuples[optIdx].iDefault++;
	       }
	       else {
		 pTuples[optIdx].iDefault = 1;
	       }
	     }
	   }
	 }
       }
       if ( optIdx!=-1 ) {
	 validOpt = True;
       }
     }
     if ( validOpt==False ) {
       fprintf(stderr, "Unrecognized option(s): %s\n", str);
       return 1;
     }
   }
   else {
     int idx = ++(opts->numArgs);
     opts->args[ idx ] = (char*)str;
   }
 }
 for (iRaw=1; iRaw<=nRaw; iRaw++) {
   oprint("rawOpts[%d]={%s}\n", iRaw, opts->rawOpts[ iRaw ]);
 }
 for (iter=1; pTuples[iter].shortOpt[0]; iter++) {
   oprint("\npTuples[%d]:\n", iter);
   oprint("%-4s  %s; %c default num: %d, default str: {%s}\n",
	  pTuples[iter].shortOpt,
	  pTuples[iter].longOpt,
	  pTuples[iter].hasArg==9 ? 'A' : 'S',
	  pTuples[iter].iDefault,
	  pTuples[iter].strDefault);
   oprint("\t%s\n", pTuples[iter].valueStr);
 }
 for (i=1; i<=opts->numArgs; i++) {
   oprint("Arg#%-2d/%d %s\n", i, argc-1, opts->args[ i ]);
 }
 return 0;
}


t_opt_tuple* find_option (const char* longAbbr)
{
 int idx=1;
 t_opt_tuple* pResult;
 const char* find;
 const char* strOpt;
 c_assert(longAbbr && longAbbr[0],"find_option()\n");
 for ( ; ; idx++) {
   strOpt = pTuples[idx].longOpt;
   if ( strOpt==NULL ) break;
   find = strstr(strOpt, longAbbr);
   if ( find==strOpt ) {
     pResult = &pTuples[ idx ];
     return pResult;
   }
 }
 return NULL;
}


/* ------------------------------------
   run_prog, main run
   ------------------------------------
*/
int cmd_head (FILE* fReport, t_opts* opts, t_http_opts* pOpt)
{
 int code = 0;
 int thisCode;
 int idx;
 char* anArg;
 t_colibri op;

 c_assert(opts && pOpt,"pOpt");
 memset(&op, 0x0, sizeof(op));
 memcpy(&op.httpOpt, pOpt, sizeof(op.httpOpt));

 if ( opts ) {
   for (idx=2; idx<=opts->numArgs; idx++) {
     t_bool isOk;
     int len;
     int resolve = 0;
     char* localURL;

     dprint("Arg: {%s}, port: %d\n", opts->args[ idx ], op.httpOpt.port);
     anArg = opts->args[ idx ];
     len = strlen( anArg );
     op.url = (t_url)malloc( len + 256 );
     strcpy(op.url, anArg);
     localURL = strchr(op.url, '/');
     if ( localURL==NULL ) {
       localURL = (char*)"\0";
     }
     else {
       localURL++;
     }
     bath_to_host( anArg, op.destHost );
     resolve = 1;
     thisCode = do_connect(fReport, &op, resolve);
     dprint("%d\tcollect('%s'), %s #%d\n",
	    thisCode,
	    op.url,
	    op.destHost,
	    op.httpCode);
     dprint("\tDest. addr: 0x%08X (host-order): 0x%08X\n",
	    op.hintDestAddr,
	    op.hintHostAddr);
     isOk = thisCode==0;
     if ( isOk ) {
       t_uint32 octets;
       int error;
       if ( op.httpOpt.oldHTTP ) {
	 snprintf((char*)mainBuffer, sizeof(mainBuffer), "GET /%s HTTP/1.0\n\n", localURL);
       }
       else {
	 snprintf((char*)mainBuffer, sizeof(mainBuffer), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", localURL, op.destHost);
       }
       if ( debugLevel >= 9 ) {
	 fprintf(fReport, "%s", mainBuffer);
       }
       error = bath_write(op.socket, &mainBuffer, strlen((char*)mainBuffer));
       if ( error==0 ) {
	 memset(mainBuffer, 0x0, sizeof(mainBuffer));
	 bath_recv(op.socket, &mainBuffer, sizeof(mainBuffer)-1, &octets);
         error = octets<=0;
	 if ( error ) {
	   dprint("Error: %s\n", op.destHost);
	 }
         else {
           mainBuffer[ octets ] = 0;
	   printf("%s", mainBuffer);
	 }
       }
       bath_socket_close( op.socket );
     }
     else {
       code = thisCode;
     }

     /* Free allocation
      */
     free(op.url);
   }
 }
 return code;
}


int run_prog (FILE* fReport, t_opts* opts)
{
 int code = 0;
 int cmdNr = 0;
 t_opt_tuple* anOpt;
 t_bool isOldHTTP = False;
 t_http_opts httpOpt;

 memset(&httpOpt, 0x0, sizeof(httpOpt));

 /* consolidate options
  */
 anOpt = find_option( "--d" );  /* ...debug */
 c_assert(anOpt, "Opts(?)# 1");
 if ( anOpt->valueStr ) {
   debugLevel = atoi( anOpt->valueStr );
 }
 anOpt = find_option( "--port" );
 c_assert(anOpt, "Opts(?)# 2");
 if ( anOpt->valueStr ) {
   oprint("%s;%s:%d;iDefault=%d;strDefault={%s},valueStr={%s}\n",
	  anOpt->shortOpt, anOpt->longOpt,
	  anOpt->hasArg,
	  anOpt->iDefault, anOpt->strDefault,
	  anOpt->valueStr);
   httpOpt.port = atoi( anOpt->valueStr );
 }
 else {
   httpOpt.port = anOpt->iDefault;
 }
 anOpt = find_option( "--version" );
 if ( anOpt->valueStr ) {
   printf("%s%s\n", VERSION, BUILD);
   return 0;
 }
 anOpt = find_option( "--HTTP" );
 isOldHTTP = anOpt->valueStr!=NULL;

 if ( (opts->numArgs)<1 ) {
 }
 else {
   cmdNr = strcmp(opts->args[ 1 ], "head")==0;
 }

 if ( cmdNr > 0 ) {
   httpOpt.oldHTTP = isOldHTTP;

   switch ( cmdNr ) {
     case 1:
       code = cmd_head( fReport, opts, &httpOpt );
       break;
     default:
       break;
     }
 }
 dprint("cmd: %d, code: %d\n", cmdNr, code);
 return code;
}


int do_connect (FILE* fReport, t_colibri* pop, int resolve)
{
 int lastOpError;
 int error = 0;
 int sck;
 t_port destPort = pop->httpOpt.port;
 struct in_addr inAddr;

 struct sockaddr_in toHostAddressData;
 int sizeofAddrData = sizeof(toHostAddressData);

 memset( (char*)&toHostAddressData, 0, sizeofAddrData );
 toHostAddressData.sin_family = AF_INET;
 if ( resolve ) {
   inAddr.s_addr = inet_addr( pop->destHost );
   resolve = inAddr.s_addr == INADDR_NONE;  /* it's a hostname */
 }
 if ( resolve ) {
   struct hostent* phostent = gethostbyname( pop->destHost );
   /* toHostAddressData.sin_addr.s_addr = inet_addr( pop->destHost ) */
   error = phostent == 0;
   dprint("Resolving '%s', error %d\n", pop->destHost, error);
   if ( error==0 ) {
     error = sizeof(inAddr) != phostent->h_length;
   }
   if ( error==0 ) {
     toHostAddressData.sin_addr.s_addr = *((unsigned long*) phostent->h_addr);
     dprint("Resolved '%s'\n", pop->destHost);
   }
 }
 else {
   error = bath_ipstr_to_addr( pop->destHost, &toHostAddressData.sin_addr );
 }
 toHostAddressData.sin_port = htons( destPort );

 if ( error ) {
   return 4;
 }
 sck = bath_socket_tcp();
 if ( sck==-1 ) {
   return -1;
 }
 memcpy(&pop->hintHostAddr, &toHostAddressData.sin_addr.s_addr, sizeof(t_uint32));
 pop->hintDestAddr = htonl( toHostAddressData.sin_addr.s_addr );
 lastOpError = connect( sck, (struct sockaddr*)&toHostAddressData, sizeofAddrData )!=0;
 if ( lastOpError ) {
   bath_socket_close( sck );
   ////perror( pop->destHost )
   pop->socket = -1;
 }
 else {
   pop->socket = sck;
 }
 return lastOpError!=0;
}


/* ------------------------------------
   main program
   ------------------------------------
*/
int main (int argc, char* argv[])
{
 int result = 0;
 t_opts* opts = (t_opts*)calloc( 1, sizeof( t_opts ) );

 if ( opts ) {
   opts->args = (char**)calloc( argc+1, sizeof(char*) );
   c_assert(opts->args!=NULL && opts->args[0]==NULL, "Basic mem.!\n");
   if ( parse_options( stderr, argc, argv, opts ) ) {
     usage();
     return 0;
   }
   result = run_prog( stderr, opts );
   free( opts->args );
   free( opts );
 }
 else {
   return -1;
 }
 return result;
}

