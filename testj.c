#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"

int
main(int argc, char* argv[])
{
  int fd, fd2;

  uchar m[1844];
  fd = open("./ddix", O_CREATE | O_RDWR);
  fd2 = open("./README", O_RDWR);

  //  uint *m;
  //  m = malloc(1024);
  // why won't this wooooooooooooork?

  printf(1, "m: %d, %d, %d\n", m, fd, fd2);
  printf(1, "%d, \n", read(fd2, m, 1844));
  printf(1, "m: %s\n", *m);
  printf(1, "%d, \n", write(fd, m, 1844));

  close(fd);
  close(fd2);

  /* appending?
   * open and mkdir both call create 
   * otherwise at fs.c level, change iupdate to work with journal
   * create also calls iupdate
   */

  exit();
  
}

