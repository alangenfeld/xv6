#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"

int
main(int argc, char* argv[])
{
  int i, fd;
  struct stat s;
  uint buf[(512 / sizeof(uint))];
  fd = open("./ddix", O_CREATE | O_RDWR);
  
  for(i = 0; i < 128 * 128 + 12; i++) {
    buf[0] = i;
    write(fd, buf, sizeof(buf)); 
    if(i%(128*16)==0) {
      fstat(fd, &s);
      printf(1, "size: %d\n", s.size);
    }
  }

  fstat(fd, &s);
  printf(1, "total size: %d\n", s.size);
  close(fd);

  fd = open("./ddix", O_CREATE | O_RDWR);
  for(i = 0; i < 128 * 128 + 12; i++) {
    read(fd, buf, sizeof(buf)); 
    if(buf[0] != i)
      printf(1, "%d != %d", i, buf[0]);
  }
  
  printf(1, "all good in the hood\n");  
  close(fd);
  exit();
}
