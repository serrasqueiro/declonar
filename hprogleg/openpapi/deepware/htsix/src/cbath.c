/*
  cbath.c -- socket handling
  the quickest HTTP robot

  (c)2018  Henrique Moreira
*/


#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cbath.h"


/* ------------------------------------
   main program
   ------------------------------------
*/
int bath_socket_tcp ()
{
 int sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 return sck;
}


int bath_socket_close (int sck)
{
 if ( sck==-1 ) return -1;
 return sck;
}


int bath_to_host (const char* strHost, char* strResult)
{
 int idx = 0;
 t_bool valid = True;
 if ( strHost==NULL || strResult==NULL ) return -1;
 memset(strResult, 0x0, MAX_HOST_CHRS);
 for ( ; idx<MAX_HOST_CHRS-1; ) {
   char u = strHost[ idx ];
   if ( u<=' ' || u=='/' ) break;
   if ( u<='*' || u>'z' ) {
     valid = False;
     idx = 0;
     break;
   }
   strResult[ idx++ ] = u;
 }
 strResult[ idx ] = '\0';
 return valid==False;  /* returns 1 on error */
}


int bath_ipstr_to_addr (const char* strHost, struct in_addr* result)
{
 if ( strHost==0 || strHost[ 0 ]==0 || result==NULL ) {
   return -1;
 }
 result->s_addr = inet_addr( strHost );
 return (result->s_addr)==0xFFFFFFFF;
}


int bath_recv (int sck, void* buf, t_uint32 bufSize, t_uint32* pOctets)
{
 int error = -1;
 int bytes;
 /* pOctets are just for statistics
  */
 if ( sck!=-1 ) {
#ifdef iDOS_SPEC
   bytes = recv(sck, buf, bufSize, 0);
#else
   bytes = read(sck, buf, bufSize);
#endif
   if ( pOctets ) {
     *pOctets = 0;
     if ( bytes > 0 ) {
       *pOctets = (t_uint32)bytes;
     }
   }
   error = bytes!=(int)bufSize;
 }
 return error;
}


int bath_send (int sck, void* buf, t_uint32 bufSize, t_uint32* pOctets)
{
 int error = -1;
 int bytes;
 if ( sck!=-1 ) {
   bytes = write(sck, buf, bufSize);
   if ( pOctets ) {
     *pOctets = 0;
     if ( bytes > 0 ) {
       *pOctets = (t_uint32)bytes;
     }
   }
   error = bytes!=(int)bufSize;
 }
 return error;
}


int bath_write (int sck, void* buf, t_uint32 bufSize)
{
 return bath_send( sck, buf, bufSize, NULL );
}

