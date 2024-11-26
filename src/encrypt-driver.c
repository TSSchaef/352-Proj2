#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "encrypt-module.h"
#include "circular-buffer.h"

int inputBufSize = -1, outputBufSize = -1;
circular_buffer inputBuf, outputBuf;
bool hitEOF = false, inputCounted = false, allEncrypted = false, outputCounted = false;
bool resetting = false;
bool hitReset = false, inputCntReset = false, encryptReset = false, 
     outputCntReset = false;

sem_t input, inputCnt, inputAvailable, output, outputCnt, outputAvailable;

pthread_mutex_t resetLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readyToReset = PTHREAD_COND_INITIALIZER;
pthread_cond_t finishedReset = PTHREAD_COND_INITIALIZER;

void reset_requested() {
    resetting = true;
    pthread_mutex_lock(&resetLock);
    pthread_cond_wait(&readyToReset, &resetLock);
    pthread_mutex_unlock(&resetLock);
	log_counts();
}

void reset_finished() {
    hitReset = false;
    inputCntReset = false;
    encryptReset = false;
    outputCntReset = false;
    resetting = false;
    pthread_cond_signal(&finishedReset);
}

void *read(void *arg){
    pthread_mutex_t lock;
    char c;
    while(1){
        if(resetting){
            hitReset = true;
            //get the next thread out of waiting
            sem_post(&input);
            pthread_mutex_lock(&readLock);
            pthread_cond_wait(&finishedReset, &readLock);
            pthread_mutex_unlock(&readLock);
        }

        //sem_wait(&inputAvailable);
        if(canAdd(inputBuf, &lock)){
            pthread_mutex_lock(&(lock));
            
            if((c = read_input()) == EOF){
                hitEOF = true;
                //get the next thread out of waiting
                sem_post(&input);
                pthread_mutex_unlock(&(lock));
                break;
            }
            push(&inputBuf, c);
            sem_post(&input);

            pthread_mutex_unlock(&(lock));
        }/* else {
            fprintf(stderr, "ERROR: Error in semaphore logic, read thread\n");
            exit(0);
        }*/
    }

    return NULL;
}

void *countInput(void *arg){
    pthread_mutex_t lock;
    bool hasWork;
    while(1){
        //sem_wait(&input);
        hasWork = canCount(inputBuf, &lock);
        if(hitEOF && !hasWork){
            inputCounted = true;
            //wake the next thread
            sem_post(&inputCnt);
            break;
        }

        if(hitReset && !hasWork){
            inputCntReset = true;
            //wake the next thread
            sem_post(&inputCnt);
            continue;
        }

        if(hasWork){
            pthread_mutex_lock(&(lock));

            count_input(countNext(&inputBuf));
            sem_post(&inputCnt);

            pthread_mutex_unlock(&(lock));
        }/* else {
            fprintf(stderr, "ERROR: Error in semaphore logic, input count thread\n");
            exit(0);
        }*/


    }
    return NULL;
}

void *encrypt_func(void *arg){
    pthread_mutex_t inputLock, outputLock;
    bool hasWork;
    while(1){
        //sem_wait(&inputCnt);
        hasWork = canPop(inputBuf, &inputLock);
        if(inputCounted && !hasWork){
            allEncrypted = true;
            break;
        }

        if(inputCntReset && !hasWork){
            encryptReset = true;
            continue;
        }

        if(hasWork){
            pthread_mutex_lock(&(inputLock));

            if(canAdd(outputBuf, &outputLock)){
                pthread_mutex_lock(&(outputLock));

                push(&outputBuf, encrypt(pop(&inputBuf)));
                sem_post(&inputAvailable);

                pthread_mutex_unlock(&(outputLock));
            } else {
                sem_post(&inputCnt);
            }

            pthread_mutex_unlock(&(inputLock));
        }/* else {
            fprintf(stderr, "ERROR: Error in semaphore logic, encrypt thread\n");
            exit(0);
        }*/
    }
    return NULL;
}

void *countOutput(void *arg){
    pthread_mutex_t lock;
    while(1){
        if(canCount(outputBuf, &lock)){
            pthread_mutex_lock(&(lock));

            count_output(countNext(&outputBuf));

            pthread_mutex_unlock(&(lock));
        }
        if(!canCount(outputBuf, &lock) && allEncrypted){
            outputCounted = true;
            break;
        }
        if(!canCount(outputBuf, &lock) && encryptReset){
            outputCntReset = true;
        }
    }
    return NULL;
}

void *write(void *arg){
    pthread_mutex_t lock;
    while(1){
        if(canPop(outputBuf, &lock)){
            pthread_mutex_lock(&(lock));
            
            write_output(pop(&outputBuf));

            pthread_mutex_unlock(&(lock));
        }
        if(!canPop(outputBuf, &lock) && outputCounted){
            break;
        }
        if(!canPop(outputBuf, &lock) && outputCntReset){
            pthread_cond_signal(&readyToReset);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 4 && argc != 6){
        fprintf(stderr, "ERROR: Must pass 3 files as commnad line arguments\n\"./encrypt in.txt out.txt log.txt\"\n");
        return -1;
    }  
    
    init(argv[1], argv[2], argv[3]); 

    if(argc == 4){
        while(inputBufSize <= 1){
            printf("Enter the input buffer size: \n");
            if(scanf("%d", &inputBufSize) != 1){
                fprintf(stderr, "ERROR: Scanf error");
            }
            if(inputBufSize <= 1){
                fprintf(stderr, "ERROR: Buffer size must be >1");
            }
        }

        while(outputBufSize <= 1){
            printf("Enter the output buffer size: \n");
            if(scanf("%d", &outputBufSize) != 1){
                fprintf(stderr, "ERROR: Scanf error");
            }
            if(outputBufSize <= 1){
                fprintf(stderr, "ERROR: Buffer size must be >1");
            }
        }
    } else {
        inputBufSize = atoi(argv[4]);
        outputBufSize = atoi(argv[5]);
    }

    init_buffer(&inputBuf, inputBufSize);
    init_buffer(&outputBuf, outputBufSize);
	
    sem_init(&input, 0, 0);
    sem_init(&inputCnt, 0, 0);
    sem_init(&inputAvailable, 0, inputBufSize - 1);
    sem_init(&output, 0, 0);
    sem_init(&outputCnt, 0, 0);
    sem_init(&outputAvailable, 0, outputBufSize - 1);
    
	/*while ((c = read_input()) != EOF) { 
		count_input(c); 
		c = encrypt(c); 
		count_output(c); 
		write_output(c); 
	} */

    pthread_t reader, input_counter, encryptor, output_counter, writer;
    pthread_create(&reader, NULL, &read, NULL);
    pthread_create(&input_counter, NULL, &countInput, NULL);
    pthread_create(&encryptor, NULL, &encrypt_func, NULL);
    pthread_create(&output_counter, NULL, &countOutput, NULL);
    pthread_create(&writer, NULL, &write, NULL);

    pthread_join(reader, NULL);
    pthread_join(input_counter, NULL);
    pthread_join(encryptor, NULL);
    pthread_join(output_counter, NULL);
    pthread_join(writer, NULL);

	printf("End of file reached.\n"); 
	log_counts();
    delete_buffer(&inputBuf);
    delete_buffer(&outputBuf);

    sem_destroy(&input);
    sem_destroy(&inputCnt);
    sem_destroy(&inputAvailable);
    sem_destroy(&output);
    sem_destroy(&outputCnt);
    sem_destroy(&outputAvailable);
}
