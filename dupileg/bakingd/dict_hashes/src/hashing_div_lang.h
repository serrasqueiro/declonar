/* hashing_div_lang.h -- (c)2019 Henrique Moreira
*/
#ifndef HASHING_DIV_LANG_x_H
#define HASHING_DIV_LANG_x_H


#define HASH_DLANG_largestPrime 4294967291UL

#define HPRIME_10P3 997		/* 10^3 */
#define HPRIME_10P4 9973	/* 10^4 */
#define HPRIME_10P5 99991	/* 10^5 */


#ifdef DEBUG
#define bprint(args...) printf(args)
#else
#define bprint(args...) ;
#endif /* ~DEBUG */

#define tFalse 0
#define tTrue 1

#define b_assert(cond,msg) { \
	if ( (cond)==0 ) { fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,msg); exit(0); } \
    }


typedef char t_bool;

typedef unsigned char t_uchar;
typedef unsigned long t_uint32;
typedef long t_int32;

typedef struct {
    t_uint32 count;
    char* str;
} t_opt_hist;

typedef struct {
    int verbose;
    int minSize;
    int lines;
    const char* strFilePath;
    int zero;
    t_opt_hist* hashgram;
} t_opt_hshow;


/* External functions
 */
extern int hist_prime_size (unsigned binSize) ;

extern char* opt_hist_add (t_opt_hist* st, const char* newStr) ;

extern t_uint32 hd_str_hash (const char* str) ;
extern t_uint32 lang_str_hash (const char* str) ;

extern char* buffer_str (t_uchar* str, int aLen) ;

#endif /* HASHING_DIV_LANG_x_H */
