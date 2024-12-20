#include "circular-buffer.h"
#include <pthread.h>

/*
    Implementation of circular buffer functionality. 
    "can..." functions need to be called/checked before calling the 
    corresponding functionality, otherwise the data structure can
    become corrupted.
*/ 

// Initialize buffer
void init_buffer(circular_buffer *buf, int size){
    buf->buffer = (element *) malloc(sizeof(element) * size);
    buf->size = size;
    buf->head = 0;
    buf->countPtr = 0;
    buf->tail = 0;
    sem_init(&(buf->canAdd), 0, size);
    sem_init(&(buf->canCount), 0, 0);
    sem_init(&(buf->canPop), 0, 0);
}

//Destroy buffer
void delete_buffer(circular_buffer *buf){
    free(buf->buffer);
    sem_destroy(&(buf->canAdd));
    sem_destroy(&(buf->canCount));
    sem_destroy(&(buf->canPop));
}

bool canAdd(circular_buffer buf){
    int value;
    sem_getvalue(&(buf.canAdd), &value);
    return value > 0;
}

void push(circular_buffer *buf, char toAdd){
    sem_wait(&(buf->canAdd));
    buf->buffer[buf->tail].c = toAdd;
    buf->tail = (buf->tail + 1) % buf->size;
    sem_post(&(buf->canCount));
}

bool canPop(circular_buffer buf){
    int value;
    sem_getvalue(&(buf.canPop), &value);
    return value > 0;
}

char pop(circular_buffer *buf){
    sem_wait(&(buf->canPop));
    char c = buf->buffer[buf->head].c;
    buf->head = (buf->head + 1) % buf->size;
    sem_post(&(buf->canAdd));
    return c;
}

bool canCount(circular_buffer buf){
    int value;
    sem_getvalue(&(buf.canCount), &value);
    return value > 0;
}

char countNext(circular_buffer *buf){
    sem_wait(&(buf->canCount));
    char c = buf->buffer[buf->countPtr].c;
    buf->countPtr = (buf->countPtr + 1) % buf->size;
    sem_post(&(buf->canPop));
    return c;
}
