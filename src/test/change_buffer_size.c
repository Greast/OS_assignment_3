#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "dm510_ioctl_commands.h"
#include <sys/ioctl.h>

int main(int argc, char const *argv[]) {

  if(argc >= 2) return 0;

  int read_pointer = open("/dev/dm510-0", O_RDONLY);
  const int error_val = ioctl(read_pointer, SET_BUFFER_SIZE, (int) strtol(argv[1], NULL, 10));

  if(error_val < 0) printf("Cant resize because too much information inside buffer.\n");
//  printf("Error (%d)\n",error_val );

  return 0;
}
