/* hashing_div_lang.c -- (c)2019 Henrique Moreira

   Basic python-like implementation of hash() function.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hashing_div_lang.h"


/* Functions start...
 */
int hist_prime_size (unsigned binSize)
{
 switch ( binSize ) {
 case 1000:
     return HPRIME_10P3;
 case 10000:
     return HPRIME_10P4;
 case 100000:
     return HPRIME_10P5;
 default:
     break;
 }
 b_assert(binSize<=0, "binSize?");
 return 0;
}


void init_opt_hist (t_opt_hist* st)
{
 b_assert(st,"?!");
 st->count = 0;
 st->str = NULL;
}


char* opt_hist_add (t_opt_hist* st, const char* newStr)
{
 int aLen;
 b_assert(newStr,"newStr");
 aLen = strlen( newStr );
 if ( st->str ) {
     aLen += strlen( st->str ) + 4;
     st->str = (char*)realloc( st->str, aLen );
     b_assert(st->str, "alloc!");
     strcat(st->str, ";");
     strcat(st->str, newStr);
 }
 else {
     b_assert(st->count==0, "count?");
     st->str = (char*)strdup( newStr );
 }
 st->count++;
 return st->str;
}


long cysw_str_hash (const char* str, int iLength)
{
 register t_uchar* p;
 register long len, x;

 len = (long)iLength;
 p = (t_uchar*)str;

 x=(*p << 7);
 while ( --len>=0 ) {
     x = (1000003*x) ^ *p++;
 }
 x ^= (long)iLength;
 if ( x==-1 ) return -2;
 return x;
}


t_uint32 hd_str_hash (const char* str)
{
 long res;
 if ( str ) {
     res = cysw_str_hash( str, strlen(str) );
 }
 else {
     res = cysw_str_hash( "", 0 );
 }
 return ((t_uint32)res) % HASH_DLANG_largestPrime;
}


t_uint32 lang_aux_hash (const char* str, int aLen)
{
 t_uint32 res;
 const int factor = 3;
 int i, j, k;
 int newLen = aLen*factor;
 char c;
 char* sec = (char*)malloc( newLen+3 );
 if ( factor>=2 ) {
     for (i=0, j=aLen, k=aLen*2; i<aLen; i++) {
	 c = str[ i ];
	 sec[ i ] = c;
	 sec[ j++ ] = -c;
	 if ( factor >= 3 ) {
	     sec[ k++ ] = j+c;
	 }
     }
     sec[ j ] += c=='o';
     sec[ i ] += c=='a';
 }
 else {
     strcpy(sec, str);
 }
#ifdef DEBUG
 {
     char* dbgStr = buffer_str( (t_uchar*)sec, newLen );
     printf("str='%s', sec='%s' (newLen=%d)\n", str, dbgStr, newLen);
     free(dbgStr);
 }
#endif /* DEBUG */
 res = cysw_str_hash( sec, newLen );
 free( sec );
 return res;
}


t_uint32 lang_str_hash (const char* str)
{
 t_uint32 h;
 int aLen;
 if ( str ) {
     aLen = strlen( str );
     switch ( aLen ) {
     case 2:
     case 3:
     case 4:
	 h = (t_uint32)lang_aux_hash( str, aLen ) % HPRIME_10P3;
	 break;
     case 5:
	 h = (t_uint32)lang_aux_hash( str, aLen ) % HPRIME_10P4;
	 break;
     default:
	 h = hd_str_hash( str );
	 break;
     }
 }
 else {
     h = hd_str_hash( str );
 }
 return h;
}


char* buffer_str (t_uchar* str, int aLen)
{
 int i;
 char* s = (char*)malloc( aLen*4 + 1 );		/* e.g. \x0a is new line (4 chars) */
 char mBuf[ 8 ];
 t_uchar u;
 b_assert(s, "s");
 s[ 0 ] = 0;
 for (i=0; i<aLen; i++) {
     u = str[ i ];
     if ( u<' ' || u>'~' ) {
	 if ( u=='\0' ) {
	     strcpy(mBuf, "\\0");
	 }
	 else {
	     sprintf(mBuf, "\\%02x", u);
	 }
     }
     else {
	 sprintf(mBuf, "%c", u);
     }
     strcat(s, mBuf);
 }
 return s;
}

