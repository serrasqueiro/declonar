/* nospell.c -- (c)2019 Henrique Moreira

   nospell - Handles dictionary of words
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nospell.h"


/* Functions start...
 */
t_uchar kind_from_str (t_uchar* wStr, t_uchar cFirst)
{
 t_uchar kind = 'n';
 if ( strchr( (char*)wStr, '-' ) ) {
     kind = 'x';
 }
 else {
     if ( cFirst>='A' && cFirst<='Z' ) {
	 kind = 'u';
     }
 }
 return kind;
}


void init_set (t_set* aSet)
{
 int z;
 b_assert(aSet, "?");
 aSet->nElems = 0;
 aSet->start = aSet->end = NULL;
 for (z=0; z<=WORD_DICT_LIM; z++) {
     aSet->bySize[ z ] = aSet->endSize[ z ] = NULL;
 }
}


void release_set (t_set* aSet)
{
 int z;
 t_word* iter;
 t_word* aNext;
 if ( aSet ) {
     for (z=0; z<=WORD_DICT_LIM; z++) {
	 for (iter=aSet->bySize[ z ]; iter; iter=aNext) {
	     aNext = iter->next;
	     free( iter );  /* 's' not allocated, and not freed! */
	 }
     }
     for (iter=aSet->start; iter; iter=aNext) {
	 aNext = iter->next;
	 free( iter->s );
	 free( iter );
     }
 }
}


t_word* set_add (t_set* aSet, t_uchar* wStr, int cache)
{
 const char* aStr = (char*)wStr;
 t_word* w;
 t_word* iter;
 int sz;
 int bogus = 0;

 b_assert(wStr, "wStr");
 b_assert(wStr[0], "wStr(2)");
 w = (t_word*)calloc( sizeof(t_word), 1 );
 b_assert(w, "Mem!");
 init_word( w );
 w->s = (char*)strdup( aStr );
 sz = strlen( aStr );
 if ( sz>=WORD_DICT_LIM ) {
     sz = WORD_DICT_LIM;
 }
 iter = aSet->start;
 if ( iter==NULL ) {
   aSet->start = w;
   aSet->end = w;
 }
 else {
   // Check if any of the words match already
   if ( cache ) {
       for (iter=aSet->bySize[ sz ]; iter; iter=iter->next) {
	   if ( strcmp(iter->s, aStr)==0 ) {
	       bogus = 1;
	       break;
	   }
       }
   }
   else {
       for ( ; iter; iter=iter->next) {
	   if ( strcmp(iter->s, aStr)==0 ) {
	       bogus = 1;
	       break;
	   }
       }
   }
 }

 if ( bogus ) {
   free( w->s );
   free( w );
   w = NULL;
 }
 else {
   aSet->nElems++;
   iter = aSet->end;
   b_assert(iter, "iter?");
   iter->next = w;
   aSet->end = w;
   /* Update kind */
   w->kind = kind_from_str( wStr, wStr[ 0 ] );
   /* Update cache */
   if ( cache ) {
       t_word* newWord = (t_word*)malloc( sizeof(t_word) );
       b_assert(newWord, "Mem (2)");
       init_word( newWord );
       iter = aSet->endSize[ sz ];
       if ( iter==NULL ) {
	   aSet->bySize[ sz ] = newWord;
	   aSet->endSize[ sz ] = newWord;
       }
       else {
	   iter->next = newWord;
	   aSet->endSize[ sz ] = newWord;
       }
       newWord->s = w->s;
       newWord->kind = w->kind;
   }
 }
 return w;
}


void init_dict (t_word_dict* wDict)
{
 int letra;
 b_assert(wDict, "!");
 wDict->total = 0;
 for (letra=0; letra<=26; letra++) {
     init_set( &wDict->rows[ letra ] );
 }
}


void release_dict (t_word_dict* wDict)
{
 int letra;
 if ( wDict ) {
     for (letra=0; letra<=26; letra++) {
	 release_set( &wDict->rows[ letra ] );
     }
 }
}


t_word* add_dict (t_word_dict* wDict, int letra, t_uchar* wStr)
{
 t_word* w = set_add( &wDict->rows[ letra ], wStr, 1 );
 return w;
}


void init_word (t_word* w)
{
 w->h = 0;
 w->kind = '.';
 w->s = NULL;
 w->next = NULL;
}

