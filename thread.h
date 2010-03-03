int thread_create( void *(*start_routine)(void*), void *arg);
 
typedef struct mutex{
  uint flag;
} mutex_t;

typedef struct cond{
  uint chan;
} cond_t;

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m); 
void cond_init(cond_t *c);
void cond_wait(cond_t *c, mutex_t *m);
void cond_signal(cond_t *c);
