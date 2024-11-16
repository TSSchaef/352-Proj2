#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct{
    char c;
    bool counted;
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

bool canAdd(const circular_buffer buf);
// MUST check canAdd before pushing (can overwrite data and break buffer)
void push(circular_buffer *buf, char toAdd);

bool canPop(const circular_buffer buf);
// MUST check canPop before pop
char pop(circular_buffer *buf);
bool isEmpty(const circular_buffer buf);

bool canCount(const circular_buffer buf);
// MUST check canCount before countNext
char countNext(circular_buffer *buf);

#endif
