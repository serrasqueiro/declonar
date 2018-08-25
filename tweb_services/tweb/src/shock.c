/* shock.c - anti shellshock environmental/ args parameter parsing
**
** (c) 2014  HM, Lisbon
*/

#include <stdio.h>
#include <string.h>

#include "shock.h"



/* aux. functions */

static void
dump_vars (FILE* fOut, char* tag, char** pArg)
{
#ifdef DEBUG_SHOCK
  int iter;

  if ( fOut ) {
    for (iter=0; pArg && *pArg; pArg++) {
      iter++;
      fprintf(fOut, "%s#%d\t%s\n", tag, iter, *pArg);
    }
  }
#endif
}



char*
find_string (char* str, char* sub)
{
 if ( str && sub ) {
   return strstr( str, sub );
 }
 return NULL;
}


/* parse environment vectors and args.
   Returns the suspicious line.
*/
char*
parse_env (char** env, char** arg, int* rejectReason)
{
 const char end_null_chr='\0';
 const char end_chr_string='~';
 char chr;
 char* str;
 char* keep;
 char* fun;
 int reject = 0;
 int iter = 0;

 static char msg[ 1024 ];

 memset(msg, 0x0, sizeof(msg));  /* Sanitize/ clean message */

 if ( env && arg ) {
   dump_vars( stdout, "e  ", env );
   dump_vars( stdout, "a  ", arg );

   /* Sanitize environment.

      Consider the following example:
	e  #1   PATH=/usr/local/bin:/usr/ucb:/bin:/usr/bin
	e  #2   SERVER_SOFTWARE=http
	e  #3   SERVER_NAME=alan
	e  #4   GATEWAY_INTERFACE=CGI/1.1
	e  #5   SERVER_PROTOCOL=HTTP/1.1
	e  #6   SERVER_PORT=8080
	e  #7   REQUEST_METHOD=GET
	e  #8   SCRIPT_NAME=/uptime.cgi
	e  #9   REMOTE_ADDR=127.0.0.1
	e  #10  HTTP_REFERER=http://localhost/
	e  #11  HTTP_USER_AGENT=() { test;}; echo; echo; date | tee -a /tmp/new_date.txt
	e  #12  HTTP_HOST=localhost
	e  #13  CGI_PATTERN=*.cgi

	The 11th argument is an attack.
   */

   for ( ; (str = *env)!=NULL; env++) {
     char* strEqual = NULL;

     if ( ++iter > 200 ) {
       sprintf(msg, "Too many env vars: %d", iter);
       reject = -1;
       break;
     }

     /* check the variable name is not weird!
      */
     for (keep=str; (chr = *keep) > ' '; keep++) {
       if ( chr=='=' ) {
	 strEqual = keep;
	 break;
       }
       if ( ((chr>='+' && chr<='z'))==0 ) {
	 snprintf(msg, 1010, "%s%s", str, strlen( str )>=1010 ? " [...]" : "");
	 reject = 1;  /* weird char in env. variable! */
       }
     }

     if ( reject ) break;

     if ( strEqual ) {
       /*
	 avoid exploitable bash before execve call;
	 how to check if your bash is vulnerable:
		env x='() { :;}; echo vulnerable' bash -c "echo this is a test"
       */
       fun = find_string( strEqual, "()" );
       if ( fun ) {
	 reject = 500;
       }
       else {
	 /* scan the whole string after '=' sign
	  */

	 int count = 0;

	 for (fun=strEqual; (chr = *fun)!=0; fun++) {
	   if ( chr=='(' ) {
	     count++;
	   }
	   else {
	     if ( chr==')' ) {
	       count--;
	       if ( count<0 ) {
		 fun = strEqual+1;
		 reject = 501;  /* closed parenthesis without opening, weird! */
		 break;
	       }
	     }
	   }
	 } /* end strEqual parsing... */

	 if ( count ) {
	   /* Additional check on parenthesis; it fits in every application.

	      Suppose the following is done:
		[guest@eagle ~]$ telnet localhost 8080
			Trying ::1...
			Trying 127.0.0.1...
			Connected to localhost.
			Escape character is '^]'.
			GET /uptime.cgi HTTP/1.1
			Host: localhost
			User-Agent: '(tiny

	      The last 'User-Agent' parenthesis is not terminated properly.
	      So, for god sake, we just reject it!
	   */

	   fun = strEqual+1;
	   reject = 502;  /* opened parenthesis without opening, weird! */
	 }
       }

       if ( reject>=500 ) {
	 snprintf(msg, 1010, "Jailed {%s}%s", strEqual, strlen( strEqual )>=1010 ? " [...]" : "");
	 strEqual[ 0 ] = end_chr_string;
	 fun[ 0 ] = end_null_chr;
	 fun[ 1 ] = end_null_chr;  /* just to be sure */
	 break;
       }
     }
   } /* end FOR environmental vars. */
 }

 if ( rejectReason ) {
   *rejectReason = reject;
 }
 return msg;
}

