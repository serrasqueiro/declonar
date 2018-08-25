/*
 * htpasswd.c: simple program for manipulating password file for NCSA httpd
 *
 * Rob McCool
 */

/* Modified 29aug97 by Jef Poskanzer to accept new password on stdin,
** if stdin is a pipe or file.  This is necessary for use from CGI.
*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <md5.h>


extern char *crypt(const char *key, const char *setting);
#define LF 10
#define CR 13

#define MAX_STRING_LEN 256

int tfd;
char temp_template[] = "/tmp/htp.XXXXXX";

void interrupted(int);


static char* md5_crypt(const char* clear);


static char * strd(char *s) {
    char *d;

    d=(char *)malloc(strlen(s) + 1);
    strcpy(d,s);
    return(d);
}

static void getword(char *word, char *line, char stop) {
    int x = 0,y;

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while((line[y++] = line[x++]));
}

static int my_getline(char *s, int n, FILE *f) {
    register int i=0;

    while(1) {
        s[i] = (char)fgetc(f);

        if(s[i] == CR)
            s[i] = fgetc(f);

        if((s[i] == 0x4) || (s[i] == LF) || (i == (n-1))) {
            s[i] = '\0';
            return (feof(f) ? 1 : 0);
        }
        ++i;
    }
}

static void putline(FILE *f,char *l) {
    int x;

    for(x=0;l[x];x++) fputc(l[x],f);
    fputc('\n',f);
}


/* From local_passwd.c (C) Regents of Univ. of California blah blah */
static unsigned char itoa64[] =         /* 0 ... 63 => ascii - 64 */
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void to64(register char *s, register long v, register int n) {
    while (--n >= 0) {
        *s++ = itoa64[v&0x3f];
        v >>= 6;
    }
}

#ifdef MPE
/* MPE lacks getpass() and a way to suppress stdin echo.  So for now, just
issue the prompt and read the results with echo.  (Ugh). */

char *getpass(const char *prompt) {

static char password[81];

fputs(prompt,stderr);
gets((char *)&password);

if (strlen((char *)&password) > 8) {
  password[8]='\0';
}

return (char *)&password;
}
#endif

static char* md5_crypt( const char* clear)
{
	static char cipher[64]; /*actualy it is something more like 36 but lets keep some spare bytes*/
	char* md5sum;
	md5_state_t state;
	md5_byte_t digest[16];
	int idx;

	md5_init(&state);
	md5_append(&state, (const md5_byte_t*) clear, strlen(clear) );
	md5_finish(&state, digest);

	sprintf(cipher, "$1$");
	md5sum = cipher + 3;

	for( idx =0; idx < 16; ++idx )
	{
		sprintf( md5sum + 2*idx,"%02x",digest[idx] );
	}
	return cipher;
}


static void
add_password( char* user, FILE* f , int md5_enabled)
    {
    char pass[100];
    char* pw;
    char* cpw;
    char salt[3];
    if ( ! isatty( fileno( stdin ) ) )
	{
	(void) fgets( pass, sizeof(pass), stdin );
	if ( pass[strlen(pass) - 1] == '\n' )
	    pass[strlen(pass) - 1] = '\0';
	pw = pass;
	}
    else
	{
	pw = strd( (char*) getpass( "New password:" ) );
	if ( strcmp( pw, (char*) getpass( "Re-type new password:" ) ) != 0 )
	    {
	    (void) fprintf( stderr, "They don't match, sorry.\n" );
	    if ( tfd != -1 )
		unlink( temp_template );
	    exit( 1 );
	    }
	}
    if( md5_enabled )
    {
        cpw = md5_crypt( pw );
    }
    else
    {
        (void) srandom( (int) time( (time_t*) 0 ) );
        to64( &salt[0], random(), 2 );
        cpw = crypt( pw, salt );
    }
    (void) fprintf( f, "%s:%s\n", user, cpw );
    }

static void usage(void) {
    fprintf(stderr,"Usage: htpasswd [-c] passwordfile username [--no-md5]\n");
    fprintf(stderr,"The -c flag creates a new file.\n");
    exit(1);
}

void interrupted(int signo) {
    fprintf(stderr,"Interrupted.\n");
    if(tfd != -1) unlink(temp_template);
    exit(1);
}

static void create_new_file(int argc , char *argv[], int md5_enabled)
{
	FILE* tfp;
    if( (argc !=5 ) && (strncmp(argv[argc-1], "-md5", strlen("-md5") == 0) ) )
    {
        fprintf( stderr, "Argument list missmatch");
        exit(1);
    }
    if(!(tfp = fopen(argv[2],"w")))
	{
	    fprintf(stderr,"Could not open passwd file %s for writing.\n", argv[2]);
	    perror("fopen");
	    exit(1);
	}
	printf("Adding password for %s.\n",argv[3]);
	add_password(argv[3],tfp, md5_enabled);
	fclose(tfp);
    exit(0);
}


int main(int argc, char *argv[]) {
    FILE *tfp,*f;
    char user[MAX_STRING_LEN];
    char line[MAX_STRING_LEN];
    char l[MAX_STRING_LEN];
    char w[MAX_STRING_LEN];
    char command[MAX_STRING_LEN];
    int found;
    int md5_enabled = 1;
    int i;

    tfd = -1;
    signal(SIGINT,(void (*)(int))interrupted);
    switch(argc)
    {
        case 5:
            if( strncmp(argv[1],"-c", strlen("-c")) )
            {
                usage();
            }
            if( 0 == strncmp(argv[argc-1], "--no-md5",strlen("-no-md5") )  )
            {
                md5_enabled =1;
            }
            create_new_file(argc,argv,md5_enabled);
            break;
        case 4:
            if( 0 == strncmp(argv[argc-1], "--no-md5",strlen("-no-md5") ) )
            {
                md5_enabled =1;
            }
            if( 0 == strncmp(argv[1],"-c", strlen("-c")) )
            {
                create_new_file(argc,argv,md5_enabled);
            }
            break;
        case 3:
            break;
        default:
            usage();
            break;
    }

    tfd = mkstemp(temp_template);
    if(!(tfp = fdopen(tfd,"w"))) {
        fprintf(stderr,"Could not open temp file.\n");
        exit(1);
    }

    if(!(f = fopen(argv[1],"r"))) {
        fprintf(stderr,
                "Could not open passwd file %s for reading.\n",argv[1]);
        fprintf(stderr,"Use -c option to create new one.\n");
        exit(1);
    }
    strcpy(user,argv[2]);

    found = 0;
    while(!(my_getline(line,MAX_STRING_LEN,f))) {
        if(found || (line[0] == '#') || (!line[0])) {
            putline(tfp,line);
            continue;
        }
        strcpy(l,line);
        getword(w,l,':');
        if(strcmp(user,w)) {
            putline(tfp,line);
            continue;
        }
        else {
            printf("Changing password for user %s\n",user);
            add_password(user,tfp, md5_enabled);
            found = 1;
        }
    }
    if(!found) {
        printf("Adding user %s\n",user);
        add_password(user,tfp, md5_enabled);
    }
    fclose(f);
    fclose(tfp);
    sprintf(command,"cp %s %s",temp_template,argv[1]);
    system(command);
    unlink(temp_template);
    exit(0);
}
