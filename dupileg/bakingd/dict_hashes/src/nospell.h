/* nospell.h -- (c)2019 Henrique Moreira
*/
#ifndef nospell_x_H
#define nospell_x_H

#include "hashing_div_lang.h"

typedef struct t_word_node {
    t_uint32 h;
    t_uchar kind;  /* n: normal-case; u: upper-case; x: other? */
    char* s;
    struct t_word_node* next;
} t_word;

typedef struct {
    t_uint32 nElems;
    t_word* start;
    t_word* end;
} t_set;

typedef struct {
    t_uint32 total;
    t_set rows[ 27 ];
} t_word_dict;

/* External functions
 */
extern void init_dict (t_word_dict* wDict) ;

extern t_word* add_dict (t_word_dict* wDict, int letra, t_uchar* wStr) ;

extern void init_word (t_word* w) ;

#endif /* nospell_x_H */
