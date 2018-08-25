
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "liblockout.h"


#define TEST_STRUCT_COUNT 4
lockout_str table[TEST_STRUCT_COUNT];
lockout_str result_table[TEST_STRUCT_COUNT];
#define PRINT_STRUCT(arg) printf("====================\nusername=%s\nhdr._size=%d\nhdr.last_failed=%ld\nnum_failed=%d\n",arg.username,arg.hdr._size,arg.hdr.last_failed,arg.hdr.num_failed);


void initTables(void)
{
 int idx=0;
 char buffer[256];
 for(idx=0; idx < TEST_STRUCT_COUNT; ++idx)
 {
	sprintf(buffer,"test%d",idx);

	(void)USER(&(table[idx]),buffer);
	table[idx].hdr.num_failed = idx + 1 ;
	table[idx].hdr.last_failed = time(0);

	(void)USER(&(result_table[idx]), buffer);
	result_table[idx].hdr.num_failed = 0;
	result_table[idx].hdr.last_failed = 0;

	sleep(1);
	PRINT_STRUCT(table[idx]);
 }
}

void destroyTable()
{
	int idx=0;
	for (idx=0;idx < TEST_STRUCT_COUNT; ++idx)
	{
		DESTROY(&table[idx]);
		DESTROY(&result_table[idx]);
	}
}

int main(int argc, char **argv)
{
	int idx=0;
	initTables();
	if( libInit() != RET_OK)
	{
		printf("libInit() failed\n");
		return 1;
	}
	/*put tests here*/
	for( idx=0; idx< TEST_STRUCT_COUNT;++idx)
	{
		if( storeData(&(table[idx])) != RET_OK )
		{
			printf("ERROR: Could not write struct:\n------------------\n");
			PRINT_STRUCT(table[idx]);
			(void)libDestroy();
			return 1;
		}
		else
		{
			printf("'%s' written successfuly \n",table[idx].username);
		}
		if( readData(&(result_table[idx])) != RET_OK )
		{
			printf("ERROR: Could not read struct: %s \n",result_table[idx].username);
			(void)libDestroy();
			return 1;
		}
		else
		{
			printf("'%s' read successfuly \n",result_table[idx].username);
			printf("%ld, %ld\n", table[idx].hdr.last_failed, result_table[idx].hdr.last_failed);
		}
	}
	/*tests end here*/
	destroyTable();
	if( libDestroy() != RET_OK)
	{
		printf("libDestroy() failed\n");
		return 1;
	}
	return 0;
}

