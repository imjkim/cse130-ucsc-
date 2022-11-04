#include "queue.h"

#include <unistd.h>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  queue_t *q = queue_new(10);
  if (q == NULL) {
    return 1;
  }

  // push a 1
  queue_push(q, (void *)1);

  // expect to pop a 1
  void *r;
  // for 10 times
  for(int i =0; i < 10; i++){
     queue_push(q, (void *)i);
  }
  for(int j = 9; j >= 0; j--){
     queue_pop(q, &r);
     if((long)r != j){
        return 1;
     }
  }
  queue_delete(&q);
  return 0;
}
