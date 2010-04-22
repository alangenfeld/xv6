#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"

int
main(int argc, char* argv[])
{
  int fd, fd2, i;
  struct stat s;
  uchar m[1844];

  fd = open("./ddix", O_CREATE | O_RDWR);
  fstat(fd, &s);
  fd2 = open("./README", O_RDWR);

  printf(1, "m: %d, %d, %d\n", m, fd, fd2);
  printf(1, "%d, \n", read(fd2, m, 1844));
  printf(1, "size: %d\n", s.size);
  for(i = 1844; i <= s.size; i+= 1844)
    read(fd, m, 1844);
  printf(1, "m: %s\n", *m);
  printf(1, "%d, \n", write(fd, m, 1844));

  close(fd);
  close(fd2);

  exit();
  
}

