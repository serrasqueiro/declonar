/* scan_ext4.c */

/* compile me using...
	gcc -Werror -Wall scan_ext4.c -o scan_ext4
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>

int run_scan (const char* name, long* myoffset) {
  unsigned char const MAGIC[2] = {0x53, 0xef};
  unsigned char const ZEROS[512] = {0};
  long long int sector = 0;
  long long int offset = 0;
  char buf[4][512];
  int fd, found = 0;
  int empty1, empty2;
  FILE* fin = NULL;

  if (name) {
      fin = fopen(name, "rb");
  }
  if (fin) {
      fd = fileno(fin);
  }
  else {
      fd = STDIN_FILENO;
  }
  while (read(fd, buf[sector&3], 512) > 0) {
    if (!memcmp(buf[sector&3] + 0x38, MAGIC, 2)) {
      printf("Found a possible ext2 partition at sector: %lld (0x%llx)\n",
	     sector-2, sector-2);
      empty1 = !memcmp(buf[(sector-2)&3], ZEROS, 512);
      empty2 = !memcmp(buf[(sector-1)&3], ZEROS, 512);
      if (empty1 && empty2) {
	  printf("\t(first two sectors are empty :)\n");
      }
      *myoffset = (long)offset;
      return 0;
    }
    sector++;
    offset += 512;
  }
  fclose(fin);
  return !found;
}

int main (int argc, char* argv[]) {
 int code;
 long offset = 0;
 const char* name = NULL;

 if (argv[1]) {
     name = argv[1];
 }
 code = run_scan(name, &offset);
 if (code == 0) {
     printf("offset: %ldd (0x%lx)\n", offset, offset);
 }
 return code;
}
