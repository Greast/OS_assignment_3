#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include "dm510_ioctl_commands.h"

int main(int argc, char const *argv[]) {
  int dev1, dev2;
  if(1 >= argc) return 0;
  dev1 = open("/dev/dm510-0", O_RDWR);
  dev2 = open("/dev/dm510-1", O_RDWR);

  const char * msg = argv[1];
  const size_t size = strlen(msg);
  printf("Writing : %s\n", msg);
  if(0 > write(dev1,msg,size)){
    printf("Message size %lu is larger then buffer size %d\n", size, ioctl(dev1,GET_BUFFER_SIZE));
    return 0;
  }

  char * buf = malloc(size*sizeof(*read));
  ssize_t a = read(dev2,buf,size);

  printf("Read : %s\n",buf);
  return 0;
}
