#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

/*
    Header file for a circular buffer data structure. The buffer must be
    initialized and destroyed and will hold exactly "size" elements (at most).
    Able to be accessed concurrently and maintained by semaphores.
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct{
    char c;
} element;

typedef struct {
    element *buffer;
    int size;
    int head;
    int tail;
    int countPtr;
    sem_t canCount;
    sem_t canAdd;
    sem_t canPop;
    
} circular_buffer;

void init_buffer(circular_buffer *buf, int size);
void delete_buffer(circular_buffer *buf);

bool canAdd(circular_buffer buf);
// MUST check canAdd before pushing (can overwrite data and break buffer)
void push(circular_buffer *buf, char toAdd);

bool canPop(circular_buffer buf);
// MUST check canPop before pop
char pop(circular_buffer *buf);
bool isEmpty(circular_buffer buf);

bool canCount(circular_buffer buf);
// MUST check canCount before countNext
char countNext(circular_buffer *buf);

#endif
