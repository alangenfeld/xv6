#include "types.h"
#include "user.h"
#include "thread.h"

struct work{
  mutex_t lk;
  cond_t c;
  volatile int x;
};

void*
con(void *w)
{
  struct work *p = w;
  printf(1, "start con\n");
  mutex_lock(&(p->lk));
  printf(1, "sleep con\n");
  cond_wait(&(p->c), &(p->lk));
  exit();
}

void*
prod(void *w)
{
  struct work *p = w;
  printf(1, "start prod\n");
  int t0 = tim();
  while((tim() - t0) < 1000);
  printf(1, "prod wake\n");
  cond_signal(&(p->c));
  exit();
}

int
main(int argc, char*argv[])
{
  struct work pkt;
  pkt.x = 0;
  mutex_init(&(pkt.lk));
  cond_init(&(pkt.c));
  thread_create(&con, &pkt);
  thread_create(&prod, &pkt);
  thread_wait();
  exit();
}
