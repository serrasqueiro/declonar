#ifndef LIBLOCKOUT_H
#define LIBLOCKOUT_H

#include <time.h>
#include <stdlib.h>

#define USER(ptr, user) fillRecord(ptr, user)
#define DESTROY(arg) free((arg)->username)

/* Structure for holding a single entry within the lockout file */
typedef struct {
	time_t last_failed;
	size_t num_failed;
	size_t _size;
} lockout_hdr;

typedef struct{
	lockout_hdr hdr;
	char *username;
} lockout_str;


/* Enum for representing return value */
typedef enum {
	RET_OK = 0,
	RET_FAIL
} RetVal;

/*Store one entry into the lockout file*/
RetVal storeData( lockout_str *ptr);
/*Read one entry from lockout file*/
RetVal readData( lockout_str *buf);
/*Remove entry from lockout file*/
RetVal delData( lockout_str *ptr);

/*Init library env*/
RetVal libInit(void);
/*Destroy library env*/
RetVal libDestroy(void);
/*calculate Lockout Time*/
time_t getLOTime(lockout_str *ptr);

/*function needed by USER macro*/
lockout_str* fillRecord(lockout_str *ptr ,const char *username);


#endif

