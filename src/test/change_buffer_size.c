#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "dm510_ioctl_commands.h"
#include <sys/ioctl.h>

int smallest_free_space(int read_pointer){
  int i;
  int min = ioctl(read_pointer,GET_BUFFER_USED_SPACE,0);
  for (i = 1; i < BUFFER_COUNT; i++) {
    int n = ioctl(read_pointer,GET_BUFFER_USED_SPACE,i);
    if(min < n) min = n;
  }
  return min;
}

int main(int argc, char const *argv[]) {

  if(1 >= argc) return 0;
  const int size = strtol(argv[1], NULL, 10);
  int read_pointer = open("/dev/dm510-0", O_RDONLY);
  const int error_val = ioctl(read_pointer, SET_BUFFER_SIZE, size);
  int min = smallest_free_space(read_pointer);
  if(error_val < 0) printf("Can't contract buffer size %d below used space %d.\n", size, min) ;
  else printf("Buffer resize to : %d bytes \n", size);

  return 0;
}
