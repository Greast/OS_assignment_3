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
  const int size = ioctl(read_pointer, GET_MAX_NR_PROC);

  size_t i;

  for (i = 0; i < size; i++) {
    open("/dev/dm510-0", O_RDONLY);
  }
  return 0;
}
