#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "dm510_ioctl_commands.h"
#include <sys/ioctl.h>

int main(int argc, char const *argv[]) {

  int read_pointer = open("/dev/dm510-0", O_RDONLY);
  const int size = ioctl(read_pointer, GET_MAX_NR_PROC, 0);
  size_t i;

  for (i = 1; i < size << 1; i++) {
    int result = open("/dev/dm510-0", O_RDONLY);
    printf("Read pointer %lu : %d ",i,  result);
    if(0 > result){
      printf("invalid pointer!\n");
      return 0;
    }
    printf("\n");
  }
  return 0;
}
