/* hashing_div_lang.h -- (c)2019 Henrique Moreira
*/
#ifndef HASHING_DIV_LANG_x_H
#define HASHING_DIV_LANG_x_H


#define HASH_DLANG_largestPrime 4294967291UL

#ifndef DEBUG
#define bprint(args...) printf(args)
#else
#define bprint(args...) ;
#endif /* ~DEBUG */

#define b_assert(cond,msg) { \
	if ( (cond)==0 ) { fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,msg); } \
    }


typedef char t_bool;

typedef unsigned char t_uchar;
typedef unsigned long t_uint32;


extern t_uint32 hd_str_hash (const char* str) ;

#endif /* HASHING_DIV_LANG_x_H */
