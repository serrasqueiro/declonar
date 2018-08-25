#ifndef T_WEB_X_H
#define T_WEB_X_H

/*
  Original coloring for 404:
<BODY BGCOLOR="#cc9999\" TEXT="#000000" LINK="#2020ff" VLINK="#4040cc">

  Original coloring for ls (directory listing):
<BODY BGCOLOR="#99cc99" TEXT="#000000" LINK="#2020ff" VLINK="#4040cc">

*/

#define HTML_BODY_404 "<body bgcolor='#e3e5ff' text='#00008b' link='#2020ff' vlink='#4040cc'>"

#define HTML_BODY_LS  "<body bgcolor='#e3e5da' text='#010101' link='#2020ff' vlink='#4040cc'>"


#ifdef DEBUG
#define dprint(args...) printf(args);
#else
#define dprint(args...) ;
#endif

#ifndef LOG_LEVEL_STAT
#define LOG_LEVEL_STAT 6	/* see also syslog.h, #define LOG_INFO 6 */
#endif

#define stats_logger(args...) syslog(LOG_LEVEL_STAT, args)

#endif /* T_WEB_X_H */


