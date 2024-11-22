#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

/*
    Header file for a circular buffer data structure. The buffer must be
    initialized and destroyed and will hold exactly "size" elements (at most)
    Mutex locks within each element allow each "slot" of the buffer to be
    accessed by different threads concurrently
*/
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct{
    char c;
    // Extra boolean with each element to denote that it has been counted
    bool counted;
    pthread_mutex_t lock;
} element;

typedef struct {
    element *buffer;
    int size;
    int head;
    int tail;
    int countPtr;
} circular_buffer;

void init_buffer(circular_buffer *buf, int size);
void delete_buffer(circular_buffer *buf);

bool canAdd(const circular_buffer buf, pthread_mutex_t *lock);
// MUST check canAdd before pushing (can overwrite data and break buffer)
void push(circular_buffer *buf, char toAdd);

bool canPop(const circular_buffer buf, pthread_mutex_t *lock);
// MUST check canPop before pop
char pop(circular_buffer *buf);
bool isEmpty(const circular_buffer buf);

bool canCount(const circular_buffer buf, pthread_mutex_t *lock);
// MUST check canCount before countNext
char countNext(circular_buffer *buf);

#endif
