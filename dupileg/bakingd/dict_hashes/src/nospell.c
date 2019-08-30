/* nospell.c -- (c)2019 Henrique Moreira

   nospell - Handles dictionary of words
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nospell.h"


/* Functions start...
 */
t_word* set_add (t_set* aSet, t_uchar* wStr)
{
 const char* aStr = (char*)wStr;
 t_word* w;
 t_word* iter;
 int bogus = 0;

 b_assert(wStr, "wStr");
 b_assert(wStr[0], "wStr(2)");
 w = (t_word*)calloc( sizeof(t_word), 1 );
 b_assert(w, "Mem!");
 init_word( w );
 w->s = (char*)strdup( aStr );
 iter = aSet->start;
 if ( iter==NULL ) {
   aSet->start = w;
   aSet->end = w;
 }
 else {
   // Check if any of the words match already
   for ( ; iter; iter=iter->next) {
     if ( strcmp(iter->s, aStr)==0 ) {
       bogus = 1;
       break;
     }
   }
   if ( bogus==0 ) {
     iter = aSet->end;
     b_assert(iter, "iter?");
     iter->next = w;
     aSet->end = w;
   }
 }
 if ( bogus ) {
   free( w->s );
   free( w );
   w = NULL;
 }
 else {
   aSet->nElems++;
 }
 return w;
}


void init_dict (t_word_dict* wDict)
{
 int letra;
 b_assert(wDict, "!");
 wDict->total = 0;
 for (letra=0; letra<=26; letra++) {
   wDict->rows[ letra ].nElems = 0;
   wDict->rows[ letra ].start = NULL;
   wDict->rows[ letra ].end = NULL;
 }
}


t_word* add_dict (t_word_dict* wDict, int letra, t_uchar* wStr)
{
 t_word* w = set_add( &wDict->rows[ letra ], wStr );
 return w;
}


void init_word (t_word* w)
{
 b_assert(w, "!");
 w->h = 0;
 w->kind = '.';
 w->s = NULL;
 w->next = NULL;
}

