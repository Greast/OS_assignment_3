#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "dm510_ioctl_commands.h"
int main(int argc, char const *argv[]) {
  int write_pointer = open("/dev/dm510-0", O_RDWR | O_NONBLOCK);
  const int size = ioctl(write_pointer, GET_BUFFER_SIZE);
  char n = 0;
  size_t i;
  for (i = 0 ; i < size; i++) {
    write(write_pointer,&n,sizeof(n));
    n++;
  }
  if(0 > write(write_pointer,&n,sizeof(n))){
    printf("Buffer is full\n");
  }
  return 0;
}
