#include "queue.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

void *thread1(void *args) {
  queue_t *q = ((void **)args)[0];
  sem_t *s = ((void **)args)[1];
  sem_t *r = ((void **)args)[2];

  // ready then wait
  sem_post(r);
  sem_wait(s);

  // push a 1
  queue_push(q, (void *)1);

  return NULL;
}

void *thread2(void *args) {
  queue_t *q = ((void **)args)[0];
  sem_t *s = ((void **)args)[1];
  sem_t *r = ((void **)args)[2];

  sem_post(r);
  sem_wait(s);

  // expect to pop a 1
  void *rv;
  queue_pop(q, &rv);
  if ((int)rv != 1) {
    // if not, then we failed
    return (void *)1;
  }
  return NULL;
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  queue_t *q = queue_new(10);
  if (q == NULL) {
    return 1;
  }
  pthread_t t1, t2;

  sem_t r, o, u;
  sem_init(&r, 0, 0);
  sem_init(&o, 0, 0);
  sem_init(&u, 0, 0);

  void *args1[3] = {q, &u, &r};
  void *args2[3] = {q, &o, &r};

  pthread_create(&t1, NULL, thread1, args1);
  pthread_create(&t2, NULL, thread2, args2);
  sem_wait(&r);
  sem_wait(&r);

  char buf[1000];
  write(1, buf, read(0, buf, 1000));
  sem_post(&o);
  write(1, buf, read(0, buf, 1000));
  sem_post(&u);

  void *rc;
  pthread_join(t1, &rc);
  if (rc != NULL)
    return 1;
  pthread_join(t2, &rc);
  if (rc != NULL)
    return 1;

  queue_delete(&q);
  return 0;
}
