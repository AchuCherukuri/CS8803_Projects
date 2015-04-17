#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        int *elements;
}Queue;

Queue * createQueue(int maxElements);

void dequeue(Queue *Q);

int front(Queue *Q);

void enqueue(Queue *Q, int element);

int sizeOfQueue(Queue *Q);

#endif /* QUEUE_H_ */
