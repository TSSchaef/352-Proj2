#include "circular-buffer.h"
#include <pthread.h>

/*
    Implementation of circular buffer functionality. 
    "can..." functions need to be called/checked before calling the 
    corresponding functionality, otherwise the data structure can
    become corrupted.
*/ 

void init_buffer(circular_buffer *buf, int size){
    buf->buffer = (element *) malloc(sizeof(element) * size);
    buf->size = size;
    buf->head = 0;
    buf->countPtr = 0;
    buf->tail = 0;
}

void delete_buffer(circular_buffer *buf){
    free(buf->buffer);
}

bool canAdd(const circular_buffer buf, pthread_mutex_t *lock){
    *lock = buf.buffer[buf.tail].lock;
    return ((buf.tail + 1) % buf.size) != buf.head;
}

void push(circular_buffer *buf, char toAdd){
    buf->buffer[buf->tail].c = toAdd;
    buf->buffer[buf->tail].counted = false;
    buf->tail = (buf->tail + 1) % buf->size;
}

bool canPop(const circular_buffer buf, pthread_mutex_t *lock){
    *lock = buf.buffer[buf.head].lock;
    return !isEmpty(buf) && buf.buffer[buf.head].counted;
}

char pop(circular_buffer *buf){
    char c = buf->buffer[buf->head].c;
    buf->head = (buf->head + 1) % buf->size;
    return c;
}

bool isEmpty(const circular_buffer buf){
    return buf.head == buf.tail;
}

bool canCount(const circular_buffer buf, pthread_mutex_t *lock){
    *lock = buf.buffer[buf.countPtr].lock;
    return buf.countPtr != buf.tail;
}

char countNext(circular_buffer *buf){
    char c = buf->buffer[buf->countPtr].c;
    buf->buffer[buf->countPtr].counted = true;
    buf->countPtr = (buf->countPtr + 1) % buf->size;
    return c;
}
