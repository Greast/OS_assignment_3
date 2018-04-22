#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char const *argv[]) {
  int read_pointer = open("/dev/dm510-0", O_RDWR);
  char msg[42];
  const int result = read(read_pointer, msg, 42);
  if(0 > result ){
    printf("Error (%d).\n", result );
  } else{
    printf("%s\n",msg);
  }
  return 0;
}
