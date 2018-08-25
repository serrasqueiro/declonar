/*std includes plus errno*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*semaphores*/
#ifdef HAVE_POSIX_SEM
#include <semaphore.h>
#else
#include <sys/sem.h>
#include <sys/ipc.h>
#endif

/*local includes*/
#include "liblockout.h"


#ifdef HAVE_POSIX_SEM
#define SEM_NAME "liblockout_sem"
#else
#define SEM_KEY 0x424242
#define SEM_COUNT 1
#define SEM_CREATE_FLAGS IPC_CREAT | IPC_EXCL
#define SEM_NUM 0
#define SEM_GET_OP (-1)
#define SEM_RELEASE_OP 1
#endif

#define LOCKOUT_FILE_DEF_STR "/tmp/lockout.dat"
#define liblockout_filename() libLock_lockout( "LOCKOUT_FILE" )
#define PERM_FLAGS S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define SEM_INIT_VAL 1

#define STR_SIZE sizeof(lockout_str)
#define HDR_SIZE sizeof(lockout_hdr)
#define STR_NULL (lockout_str*)0

#define ASSIGN_HDR( dst, src) {(*dst).hdr.last_failed = (*src).hdr.last_failed; (*dst).hdr.num_failed = (*src).hdr.num_failed;}
#define WRITE_HDR(arg) write(lockout_fd, (void*)arg, HDR_SIZE)
#define WRITE_USERNAME(arg) write(lockout_fd, (void*) (*arg).username, (*arg).hdr._size)

#define PERROR(arg) __print_perror(__FUNCTION__,__LINE__,arg)



#if !defined(__GNU_LIBRARY__) || defined(_SEM_SEMUN_UNDEFINED)
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};
#endif



#ifdef HAVE_POSIX_SEM
#define SEM_RELEASE(arg)  if( sem_post(lockout_sem) < 0 ) \
	{ \
		PERROR("sem_post()"); \
		return arg;\
	}
#else
#define SEM_RELEASE(arg) {\
		struct sembuf op_s;\
		int rc = -1;\
		op_s.sem_num = SEM_NUM;\
		op_s.sem_op = SEM_RELEASE_OP;\
		op_s.sem_flg = 0;\
		rc = semop( lockout_sem, &op_s, 1);\
		if( rc < 0) \
		{ \
			PERROR("semop(RELEASE)");\
			return arg;\
		}\
	}
#endif

#ifdef HAVE_POSIX_SEM
#define SEM_GET(arg) if ( sem_wait(lockout_sem) < 0 )\
	{\
		PERROR("sem_wait()");\
		return arg;\
	}
#else
#define SEM_GET(arg) {\
		struct sembuf op_s;\
		int rc = -1;\
		op_s.sem_num = SEM_NUM;\
		op_s.sem_op = SEM_GET_OP;\
		op_s.sem_flg = 0;\
		rc = semop( lockout_sem, &op_s, 1);\
		if( rc < 0) \
		{ \
			PERROR("semop(GET)");\
			return arg;\
		}\
	}
#endif


/*global variables*/
#ifdef HAVE_POSIX_SEM
sem_t *lockout_sem = (sem_t*)0;
#else
int lockout_sem = -1;
#endif
int lockout_fd = 0;


/* =============================static functions */

#define PRINT_BUFFER_SIZE 512


static const char* libLock_lockout (const char* strEnvVar)
{
	char* strName = LOCKOUT_FILE_DEF_STR;
	const char* strAlt = NULL;
	if ( strEnvVar && strEnvVar[ 0 ] ) {
	  strAlt = getenv( strEnvVar );  /* env var LOCKOUT_FILE exists? */
	  if ( strAlt && strAlt[ 0 ] ) {
	    strName = strdup( strAlt );
	  }
	}
	return strName;
}

#define LOCKOUT_FILE_DEF_STR "/tmp/lockout.dat"
#define liblockout_filename() libLock_lockout( "LOCKOUT_FILE" )

static void __print_perror(const char *func, int line, const char *string)
{
	if( !string )
	{
		/*null passed as parameter -> do nothing*/
		return;
	}

	{ /* {}  to limit the variable scope*/
		char buffer[PRINT_BUFFER_SIZE];
		sprintf(buffer,"liblockout::%s::%d::%s",func,line,string);
		perror(buffer);
	}

}

static lockout_str* getNextRecord(int fd, lockout_str *buf, off_t *offset, off_t *found)
{
	void *username_ptr = (void*) 0;
	int rc = 0;
	off_t rc_off = 0;

	*found = (off_t) 0;
	/*hopefully the operation will not be carried out frequently so no need for mmap/shm...*/
	SEM_GET(STR_NULL);
	rc_off = lseek(fd, *offset, SEEK_SET );
	if(  rc_off == (off_t)-1 )
	{
		PERROR("lseek()");
		SEM_RELEASE(STR_NULL);
		return STR_NULL;
	}
	rc = read(fd, (void*) &(buf->hdr) , HDR_SIZE);
	if( rc <0)
	{
		/*error: could not read the record header file may be corrupted  */
		PERROR("read()");
		SEM_RELEASE(STR_NULL);
		return STR_NULL;
	}
	if( rc == 0 )
	{
		/*end of file*/
		SEM_RELEASE(STR_NULL);
		return	STR_NULL;
	}
	if(buf->hdr._size != 0 )
	{
		/*allocate memory for username*/
		username_ptr = malloc(buf->hdr._size+1);
		if( !username_ptr )
		{
			/*error allocating*/
			SEM_RELEASE(STR_NULL);
			return STR_NULL;
		}
		buf->username = (char*) username_ptr;
		rc_off = lseek(fd, *offset + HDR_SIZE, SEEK_SET );
		if(  rc_off == (off_t)-1 )
		{
			PERROR("lseek()");
			SEM_RELEASE(STR_NULL);
			return STR_NULL;
		}
		rc = read(fd, (void*)buf->username, buf->hdr._size);
		if( rc  <= 0)
		{
			PERROR("read()");
			DESTROY(buf);
			SEM_RELEASE(STR_NULL);
			return STR_NULL;
		}
		buf->username[buf->hdr._size]='\0'; /*just to be able to sleep at night*/
	}
	else
	{
		buf->username = (char*) 0;
	}
	SEM_RELEASE(STR_NULL);
	/*advance the file pointer*/
	*found = *offset;
	*offset += HDR_SIZE + buf->hdr._size;
	return buf;

}

/*returns pointer to the field in the file on success, STR_NULL otherwise*/

static off_t findRecord(int fd, lockout_str *record)
{
	lockout_str buf;
	off_t ptr = (off_t)0;
	off_t found = (off_t)0;
	while( getNextRecord(fd, &buf ,&ptr, &found))
	{
		if ( record->hdr._size != buf.hdr._size )
		{
			/*sizes of username fields are different - no need to strncmp*/
			DESTROY(&buf);
			continue;
		}
		if( record->hdr._size == 0) /*for usernames not listed in /etc/passwd */
		{
			ASSIGN_HDR(record,&buf);
			DESTROY(&buf);
			return found;
		}
		if( strncmp( record->username, buf.username, record->hdr._size) == 0 )
		{
			ASSIGN_HDR(record,&buf);
			DESTROY(&buf);
			return found;
		}
		DESTROY(&buf);
	}
	return -1;
}

/* =============================global functions*/

RetVal storeData( lockout_str *ptr)
{
	off_t tmp = (off_t)0;
	lockout_str key; /*this wil be initialized later*/
	off_t file_pos = 0;
	int rc = 0;

	(void)USER(&key, ptr->username);

	if( lockout_fd < 0 )
	{
		return RET_FAIL;
	}
	tmp = findRecord(lockout_fd, &key);
	if( tmp == (off_t) -1 )
	{
		SEM_GET(RET_FAIL)
		/*file does not contain such data ->  write new one*/
		file_pos = lseek(lockout_fd, 0, SEEK_END);
		if ( file_pos == (off_t)-1 )
		{
			PERROR("lseek()");
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
		rc=WRITE_HDR(ptr);
		if( rc < 0 )
		{
			PERROR("write(HDR)");
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
		rc=WRITE_USERNAME(ptr);
		if(rc < 0)
		{
			PERROR("write(USERNAME)");
			/*header written to file already ... need to truncate file*/
			if( ftruncate(lockout_fd, file_pos) < 0 )
			{
				PERROR("ftruncate()");
			}
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
		SEM_RELEASE(RET_FAIL);
		return RET_OK;
	}

	SEM_GET(RET_FAIL);
	if ( lseek(lockout_fd,  tmp, SEEK_SET) == (off_t)-1 )
	{
		PERROR("lseek()");
		SEM_RELEASE(RET_FAIL);
		return RET_FAIL;
	}
	rc = WRITE_HDR(ptr);
	if ( rc < 0 )
	{
		PERROR("write(UPDATE)");
		SEM_RELEASE(RET_FAIL);
		return RET_FAIL;
	}
	SEM_RELEASE(RET_FAIL);
	return RET_OK;

}

RetVal readData( lockout_str *buf)
{
	if(lockout_fd < 0 )
	{
		return RET_FAIL;
	}
	if ( findRecord(lockout_fd, buf) == (off_t) -1 )
	{
		/*error finding record in file*/
		return RET_FAIL;
	}
	/*entry found*/
	return RET_OK;
}

RetVal delData( lockout_str *ptr)
{
	off_t pos = (off_t)0;
	off_t tmp = (off_t)0;
	off_t record_offset = (off_t)0;
	size_t buffer_size = 0;
	char *buf_tmp = (char*)0;
	off_t new_filesize = (off_t)0;

	if (lockout_fd < 0)
	{
		return RET_FAIL;
	}
	pos = findRecord(lockout_fd, ptr);
	if (pos == 0)
	{
		/*record not in the file or error occured*/
		return RET_FAIL;
	}
	record_offset = pos + HDR_SIZE + ptr->hdr._size;
	SEM_GET(RET_FAIL);
	tmp = lseek( lockout_fd, 0, SEEK_END);
	if (tmp < 0)
	{
		PERROR("lseek()");
		SEM_RELEASE(RET_FAIL);
		return RET_FAIL;
	}
	buffer_size = (size_t)(tmp > record_offset ? tmp - record_offset : 0);
	if ( buffer_size > 0 )
	{
		/*record in the "middle" of a file*/
		buf_tmp = (char*) malloc(buffer_size);
		if(!tmp)
		{
			/*could not allocate memory*/
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
		if( read(lockout_fd, (void*)buf_tmp, buffer_size) < 0)
		{
			/*some error - ups*/
			PERROR("read()");
			free(buf_tmp);
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
		if ( lseek(lockout_fd, (off_t) pos, SEEK_SET) == (off_t)-1)
		{
			PERROR("lseek()");
			free(buf_tmp);
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
		if( write(lockout_fd, (const void*)buf_tmp, buffer_size ) < 0 )
		{
			PERROR("write()");
			free(buf_tmp);
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
		new_filesize = tmp - (HDR_SIZE + ptr->hdr._size);
		if( ftruncate(lockout_fd, new_filesize) < 0)
		{
			PERROR("ftruncate()");
			free(buf_tmp);
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
	}
	else
	{
		/*end of file*/
		if( ftruncate(lockout_fd, (off_t)pos )  < 0 )
		{
			PERROR("ftruncate()");
			SEM_RELEASE(RET_FAIL);
			return RET_FAIL;
		}
	}
	SEM_RELEASE(RET_FAIL);
	return RET_OK;
}

RetVal libInit(void)
{
	const char* strLockoutFilename = NULL;
#ifdef HAVE_POSIX_SEM
	sem_t *sem = (sem_t*)0;
	sem = sem_open( SEM_NAME, O_CREAT , PERM_FLAGS , SEM_INIT_VAL);
	if (sem == SEM_FAILED)
	{
		/*error occurred*/
		PERROR("sem_open()");
		return RET_FAIL;
	}
	/*semaphore opened/created*/
	lockout_sem = sem;
#else
	int sem_fd = -1;
	union semun semctl_arg;
	int rc = -1;

	sem_fd = semget( SEM_KEY, SEM_COUNT, SEM_CREATE_FLAGS | PERM_FLAGS );
	if( sem_fd >= 0 )
	{
		/*sem created -> initialize*/
		lockout_sem = sem_fd;
		semctl_arg.val = SEM_INIT_VAL;
		/* Moderate to Low risk race condition here due to sysV semaphore specification.
		*  Actions taken: NONE
		*  Explanation: there is no 'low effort' solution for this and the occurrence
		*               possibility is low, so it has been decided this issue to be treated as
		*               negligible
		*/
		rc = semctl( lockout_sem, SEM_NUM, SETVAL, semctl_arg);
		if( rc < 0 )
		{
			PERROR("semgctl()");
			/*TODO: is ther a need to rmid here ?*/
			return RET_FAIL;
		}
	}
	else if ( errno == EEXIST )
	{
		/*sem exists -> no need to init it, just repeat the get commnand*/
		sem_fd = semget( SEM_KEY, SEM_COUNT, PERM_FLAGS);
		if ( sem_fd < 0 )
		{
			/*regardless of the errno value, this is an error*/
			PERROR("semget()");
			return RET_FAIL;
		}
		lockout_sem = sem_fd;
	}
	else
	{
		PERROR("semget()");
		return RET_FAIL;
	}
#endif
	strLockoutFilename = liblockout_filename();
	lockout_fd = open( strLockoutFilename, O_RDWR|O_CREAT|O_NONBLOCK, PERM_FLAGS ); /*non block can be safely used here as /tmp resides in ram */
	if ( lockout_fd < 0 )
	{
		PERROR( strLockoutFilename );  /* or ... "open()" */
#ifdef HAVE_POSIX_SEM
		/*TODO: do sem_close() here ... */
#endif
		fprintf(stderr, "Hint:\nDefine LOCKOUT_FILE as /tmp/username.dat\n");
		return RET_FAIL;
	}
	return RET_OK;
}

RetVal libDestroy(void)
{
#ifdef HAVE_POSIX_SEM
	int sem_val=0;
	if ( sem_getvalue(lockout_sem, &sem_val) < 0 )
	{
		/*error*/
		PERROR("sem_getvalue()");
		return RET_FAIL;
	}
	if (sem_val <= 0 )
	{
		/* there a still processes waiting for the semaphore */
		/* TODO: return RET_FAIL here or try to fix something on the fly? */
		fprintf(stderr, "Cannot destroy lib env -> %d processes still waiting for semaphore \n", abs(sem_val));
		return RET_FAIL;
	}
	/*  semaphore is not taken, we are free to destroy it*/
	/*  sem_close() used here, because of possible thttpd restarts ...
	 *	the semaphore should be removed after reboot as /tmp is cleared during reset
	 */
	if( sem_close(lockout_sem) < 0 )
	{
		PERROR("sem_close()");
		return RET_FAIL;
	}
#else
	/*the sem is not removed from the system on purpose in case of sysv semaphores*/
#endif
	/*close lockout file*/
	if( close(lockout_fd) < 0)
	{
		PERROR("close()");
		return RET_FAIL;
	}
	lockout_fd = -1;
	/*env done for -> exit from func with happy news*/
	return RET_OK;
}

lockout_str* fillRecord(lockout_str *ptr, const char *username)
{
	char *tmp = (char*) 0;
	size_t username_size = 0;
	if( !username )
	{
		/*this is not good -> exit with error*/
		return STR_NULL;
	}
	username_size = strlen(username);
	if(username_size == 0 )
	{
		/*this is not goot -> exit with error*/
		printf("username is empty string\n");
		/*return STR_NULL;*/
	}
	++username_size; /*additional byte for '\0'*/
	tmp = malloc(username_size);
	if(!tmp)
	{
		return STR_NULL;
	}
	ptr->username = tmp;
	ptr->hdr._size = username_size;
	(void)strncpy(ptr->username, username, username_size); /*strncpy takes care about the '\0'*/

	return ptr;
}

#define TRESHOLD_TRIES 5
#define CONST_A 300
#define CONST_B 120
#define NORMAL_TIMEOUT 0

time_t getLOTime(lockout_str *ptr)
{
	if(!ptr)
	{
		return 0;
	}
	if( ptr->hdr.num_failed >= TRESHOLD_TRIES )
	{
		return CONST_A + CONST_B*(ptr->hdr.num_failed - TRESHOLD_TRIES);
	}
	return NORMAL_TIMEOUT;
}


