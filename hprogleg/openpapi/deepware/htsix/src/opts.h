/* opts.h

   (c)2018  Henrique Moreira
*/

#define VERSION "0.01"
#define BUILD "\
 12\
"

#include "cbath.h"

#ifdef DEBUG
#define oprint(args...) printf(args)
#else
#define oprint(args...) ;
#endif

#define dprint(args...) { \
	if (debugLevel) { fprintf(fReport, args); }	\
    }



typedef struct _t_opts {
    t_bool isHTTPS;
    short numArgs;
    short numOptions;
    const char* rawOpts[ 128 ];
    char** args;
} t_opts;


typedef struct _t_opt_tuple {
    char shortOpt[ 4 ];
    const char* longOpt;
    short hasArg;  /* 9: by convention, value in next arg. */
    int iDefault;
    const char* strDefault;
    char* valueStr;
} t_opt_tuple;

