#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"

int
main(int argc, char* argv[])
{
  struct stat s;
  char buf[6000];
  int fd = open("./dix", O_CREATE | O_RDWR);

  fstat(fd, &s);
  printf(1, "size: %d\n", s.size);
  printf(1, "fd: %d\n", fd);

  int result = check(fd, 1);
  printf(1, "offset should not be not be cached/exist\n");
  printf(1, "result: %d\n", result);

  write(fd, buf, 10);
  result = check(fd, 1);
  printf(1, "offset should be cached after a small write\n");
  printf(1, "result: %d\n", result);
  
  write(fd, buf, 6000);
  result = check(fd, 1);
  printf(1, "offset should not be cached after a BIG write\n");
  printf(1, "result: %d\n", result);
  
  result = check(fd, 5999);
  printf(1, "offset should be cached after a BIG write\n");
  printf(1, "result: %d\n", result);

  close(fd);

  fd = open("./dix", O_RDONLY);
  read(fd, buf, 4);
  result = check(fd, 1);
  printf(1, "offset should be cached after a small read\n");
  printf(1, "result: %d\n", result);
  read(fd, buf, 5898);
  close(fd);

  exit();
}
