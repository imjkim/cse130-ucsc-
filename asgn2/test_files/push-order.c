#include "queue.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PUSH_PER_THREAD 10

void *thread1(void *args) {
  queue_t *q = ((void **)args)[0];
  int id = (int)(((void **)args)[1]);

  for (int i = 0; i < PUSH_PER_THREAD; i++) {
    if (!queue_push(q, (void *)(intptr_t)(i + (id << 8)))) {
      break;
    }
  }

  return NULL;
}

void *thread2(void *args) {
  queue_t *q = ((void **)args)[0];
  int id = (int)(((void **)args)[1]);

  int clock[id];
  for (int i = 0; i < id; i++) {
    clock[i] = 0;
  }

  void *rv;
  for (int i = 0; i < id * PUSH_PER_THREAD; i++) {
    if (!queue_pop(q, &rv)) {
      return (void *)1;
    }
    if ((((int)rv) & 0xff) != clock[((int)rv) >> 8]++) {
      return (void *)1;
    }
  }
  for (int i = 0; i < id; i++) {
    if (clock[i] != PUSH_PER_THREAD) {
      return (void *)1;
    }
  }

  return NULL;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    return 2;
  }

  int tc = atoi(argv[1]);
  queue_t *q = queue_new(tc * PUSH_PER_THREAD);
  if (q == NULL) {
    return 1;
  }
  pthread_t t1, t2[tc];

  void *args1[2] = {q, (void *)(intptr_t)tc};
  void *args2[2 * tc];

  pthread_create(&t1, NULL, thread2, args1);
  for (int i = 0; i < tc; i++) {
    args2[i * 2 + 0] = q;
    args2[i * 2 + 1] = (void *)(intptr_t)i;
    pthread_create(t2 + i, NULL, thread1, args2 + (2 * i));
  }

  void *rc;
  pthread_join(t1, &rc);
  if (rc != NULL)
    return 1;
  for (int i = 0; i < tc; i++) {
    pthread_join(t2[i], &rc);
    if (rc != NULL)
      return 1;
  }

  queue_delete(&q);
  return 0;
}
