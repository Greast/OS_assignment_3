#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "dm510_ioctl_commands.h"
int main(int argc, char const *argv[]) {
  for (size_t i = 0; i < DEVICE_COUNT; i++) {
    int write_pointer = open("/dev/dm510-0", O_RDWR);
    printf("Write pointer %lu : %d ",i,  write_pointer);
    if(0 > write_pointer){
      printf("invalid pointer!\n");
      return 0;
    }
    printf("\n");
  }
  return 0;
}
