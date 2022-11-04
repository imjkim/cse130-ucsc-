#include "queue.h"

#include <unistd.h>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  queue_t *q = queue_new(50);
  if (q == NULL) {
    return 1;
  }

  // push a 1
  queue_push(q, (void *)1);

  // expect to pop a 1
  void *r;
  for(int i = 0; i < 49; i++){
      queue_push(q, (void *)1);
  }
  for(int j = 0; j < 49; j++){
      queue_pop(q, &r);
      if ((long)r != 1) {
       // if not, then we failed
         return 1;
      }
  }
  queue_delete(&q);
  return 0;
}
