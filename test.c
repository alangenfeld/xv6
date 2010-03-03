#include "types.h"
#include "user.h"
#include "thread.h"

mutex_t lock;

void*
WTF(void *p) // work time fun
{
  mutex_lock(&lock);
  printf(1, "thread %d[%x]\n", *(int*)p, p);
  mutex_unlock(&lock);
  exit();
}

void*
work1(void *p)
{
  int a, b, c;
  mutex_lock(&lock);
  printf(1, "caller thread: (%d) %x [%x]\n",*(int*)p, p, &p);
  mutex_unlock(&lock);
  a = thread_create(WTF, &a);
  b = thread_create(WTF, &b);
  c = thread_create(WTF, &c);
  thread_wait();
  exit();
}

int
main(int argc, char* argv[])
{
  int a, b, c;
  mutex_init(&lock);
  printf(1, "r: %x a: %x\n", &(WTF), &a);
  printf(1, "WTF: %x, &WTF: %x\n", WTF, &WTF);
  a = thread_create(&WTF, &a);
  thread_wait();
  b = thread_create(WTF, &b);
  c = thread_create(work1, &c);
  thread_wait();
  exit();
}
