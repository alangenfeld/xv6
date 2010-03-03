#include "types.h"
#include "user.h"
#include "thread.h"

int queue[5];
int loops, numfilled, MAX, index;
cond_t empty, fill;
mutex_t mutex;

void *producer(void *arg) {
  int i,j;
  for (i = 0; i < loops/2; i++) {
    mutex_lock(&mutex);           

    while (index == MAX) {
      cond_signal(&fill);           
      printf(1, "prod %d waiting\n", (int*)arg);
      cond_wait(&empty, &mutex);
    }
    
    queue[index++] = i;                 
    
    printf(1, "prod: %d, queue:", (int*)arg);
    for(j = 0; j < MAX; j++)
      printf(1, "[%d] = %d, ", j, queue[j]); 
    printf(1, "\n");
    
    mutex_unlock(&mutex);         
  }
  cond_signal(&fill);
  exit();
}

void *consumer(void *arg) {
  int i;
  for (i = 0; i < loops/2; i++) {
    mutex_lock(&mutex);
      
    while (index == 0) {
      cond_signal(&empty);
      printf(1, "cons %d waiting\n", (int*)arg);
      cond_wait(&fill, &mutex);
    }
      
    int tmp = queue[--index];
    printf(1, "cons: %d, %d\n", (int*)arg, tmp);
      
    mutex_unlock(&mutex);
  }
  cond_signal(&empty);
  exit();
}

int
main(int argc, char*argv[])
{
  loops = 200;
  MAX = 5;
  index = 0;
  mutex_init(&mutex);
  cond_init(&empty);
  cond_init(&fill);
  int c1 = thread_create(&consumer, &c1);
  int c2 = thread_create(&consumer, &c2);
  int p1 = thread_create(&producer, &p1);
  int p2 = thread_create(&producer, &p2);
  thread_wait();
  exit();
}
