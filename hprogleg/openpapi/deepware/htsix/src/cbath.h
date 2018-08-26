#ifndef CBATH_X_H
#define CBATH_X_H


#include <sys/socket.h>
#include <arpa/inet.h>


#ifndef False
#define False 0
#endif

#ifndef True
#define True 1
#endif

#define c_assert_ErrorCode 4
#define c_unassert(args...) { \
	fprintf(stderr, "%s:%d ", __FILE__, __LINE__); fprintf(stderr, args); exit(c_assert_ErrorCode); \
    }
#define c_assert(x,args...) { \
	if ((x)==0) { fprintf(stderr, "%s:%d ", __FILE__, __LINE__); fprintf(stderr, args); exit(c_assert_ErrorCode); } \
    }

#define INT_MAX_32bit 2147483647
#define INT_MIN_32bit -2147483648

#define MAX_HOST_CHRS 256


typedef short t_bool;
typedef unsigned char t_uchar;
typedef unsigned short t_uint16;
typedef unsigned t_uint32;
typedef t_uint32 t_ipv4;

typedef short t_debug_level;
typedef unsigned short t_port;
typedef char* t_url;
typedef short t_http_code;


typedef struct {
    t_port port;
    t_bool oldHTTP;
    const char* strHost;  /* 'Host: %s' in header */
} t_http_opts;

typedef struct {
    int socket;
    char destHost[MAX_HOST_CHRS];
    t_url url;
    t_http_code httpCode;
    t_ipv4   hintDestAddr;
    t_uint32 hintHostAddr;  /* host-order */
    t_http_opts httpOpt;
} t_colibri;


/* ------------------------------------
   External functions
   ------------------------------------
*/
extern int bath_socket_tcp () ;
extern int bath_socket_close (int sck) ;

extern int bath_to_host (const char* strHost, char* strResult) ;

extern int bath_ipstr_to_addr (const char* strHost, struct in_addr* result) ;

extern int bath_recv (int sck, void* buf, t_uint32 bufSize, t_uint32* pOctets) ;

extern int bath_send (int sck, void* buf, t_uint32 bufSize, t_uint32* pOctets) ;
extern int bath_write (int sck, void* buf, t_uint32 bufSize) ;


#endif //CBATH_X_H

