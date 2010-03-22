#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"

int
main(int argc, char* argv[])
{
  int i;
  struct stat s;
  uint buf[512 / sizeof(uint)];
  int fd = open("./ddix", O_CREATE | O_RDWR);
  fstat(fd, &s);
  printf(1, "size: %d\n", s.size);
  
  for(i = 0; i < 16 * 16 + 12; i++) {
    buf[0] = i;
    write(fd, buf, 512); 
    printf(1, "size: %d\n", s.size);
  }
  
  //  printf(1, "size: %d\n", s.size);
  close(fd);
  exit();
}
