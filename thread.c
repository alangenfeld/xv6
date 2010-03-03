#include "types.h"
#include "user.h"
#include "x86.h"
#include "thread.h"

void mutex_init(mutex_t *m) {
  // 0 indicates that lock is available, 1 that it is held by a thread
  m->flag = 0;
}

void mutex_lock(mutex_t *m)
{
  while (xchg(&m->flag, 1) == 1)
    ; // spin-wait (do nothing)
}

void mutex_unlock(mutex_t *m)
{
  m->flag = 0;
} 

void cond_init(cond_t *c)
{
  c->chan = 0;
  c->cnt = 0;
}

void cond_wait(cond_t *c, mutex_t *m)
{
  mutex_unlock(m);
  cond_sleep(&(c->chan) + c->cnt++);
  mutex_lock(m);
}

void cond_signal(cond_t *c)
{
  if(c->chan < c->cnt)
    cond_wake(&(c->chan) + c->chan++);
}

int thread_create( void *(*start_routine)(void*), void *arg)
{
  void *sp = malloc(1024);
  int pid = thread(sp, (int)start_routine, (int)arg);
  return pid;
}
