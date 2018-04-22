#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char const *argv[]) {
  int write_pointer = open("/dev/dm510-0", O_RDWR);
  int error_pointer = open("/dev/dm510-0", O_RDWR);
  printf("Error (%d)\n",error_pointer );
  return 0;
}
