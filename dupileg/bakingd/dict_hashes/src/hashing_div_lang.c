/* hashing_div_lang.c -- (c)2019 Henrique Moreira

   Basic python-like implementation of hash() function.
*/

#include <string.h>

#include "hashing_div_lang.h"


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

