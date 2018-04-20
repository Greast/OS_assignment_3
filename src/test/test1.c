#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char const *argv[]) {
  int read_pointer = open("/dev/dm510-0", O_RDWR);
  char * msg = "hello";
   


  int write_pointer = open("/dev/dm510-1", O_RDWR);
  return 0;
}
