#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  int dev1, dev2;
  dev1 = open("/dev/dm510-0", O_RDWR);
  dev2 = open("/dev/dm510-1", O_RDWR);

  const char * hello  = "hello";
  write(dev1,hello,strlen(hello));

  char world[20];
  ssize_t a = read(dev2,world,strlen(hello));

  printf("%li : %c\n",a, *world);
  return 0;
}
