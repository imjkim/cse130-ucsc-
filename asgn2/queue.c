#include <pthread.h>
#include <stdlib.h>
#include "queue.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
typedef void *QueueValue;
pthread_mutex_t queueLock;
pthread_cond_t queueFull;
pthread_cond_t queueEmpty;

queue_t *myQueue;
typedef struct sQueueItem{
    QueueValue value;
} QueueItem;
// queue has FIFO properties and should support multi producer and multi consumer
struct queue{
    QueueItem *items;
    int num;
    int maxSize;
    int head;
    int tail;
};

// Allocates a queue with size `size`
queue_t *queue_new(int size){
    struct queue *new = (struct queue*)malloc(sizeof(queue_t));
    QueueItem *items = malloc(sizeof(QueueItem) * size);
    new->items = items;
    new->maxSize = size;
    new->num = 0;
    new->head = 0;
    new->tail = 0;
    return new;
}

// frees a queue (should assume the queue is empty)
void queue_delete(queue_t **q){
    pthread_mutex_lock(&queueLock);
    free((*q)->items);
    free(*q);
    pthread_mutex_unlock(&queueLock);
    
}

// add an element to the queue
// blocks if queue is full
bool queue_push(queue_t *q, void *elem){
    bool ret = true;
    pthread_mutex_lock(&queueLock);
    if(q == NULL){
        return false;
    }
    while(q->maxSize <= q->num){
        pthread_cond_wait(&queueFull, &queueLock);
        //block
    } 
    q->items[q->tail].value = elem;
        //memcpy(q->items[q->tail].value, elem, sizeof(elem));
    q->tail =(q->tail + 1) % q->maxSize;
    q->num++;
    if(q->num == 1){
        pthread_cond_signal(&queueEmpty);
    }
    pthread_mutex_unlock(&queueLock);
    return ret;
}

// remove an element from the queue
// blocks if the queue is empty
bool queue_pop(queue_t *q, void **elem){
    bool ret = true;
    pthread_mutex_lock(&queueLock);
    while(q->num == 0){
        pthread_cond_wait(&queueEmpty, &queueLock);

    } 
    *elem = q->items[q->head].value;
       //memcpy(elem, q->items[q->head].value, sizeof(*elem));
    q->head = (q->head +1) %q->maxSize;
    q->num--;
    if(q->num == (q->maxSize -1)){
        pthread_cond_signal(&queueFull);

    }
    pthread_mutex_unlock(&queueLock);
    return ret;
}
