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

sem_t readThd, inputCntThd, encryptInThd, encryptOutThd, outputCntThd, writeThd;
sem_t reset, resetDone;

void reset_requested() {
    resetting = true;

    //wait for each of the threads
    sem_wait(&reset);
    sem_wait(&reset);
    sem_wait(&reset);
    sem_wait(&reset);
    sem_wait(&reset);

	log_counts();
}

void reset_finished() {
    hitReset = false;
    inputCntReset = false;
    encryptReset = false;
    outputCntReset = false;
    resetting = false;

    sem_post(&resetDone);
    sem_post(&resetDone);
    sem_post(&resetDone);
    sem_post(&resetDone);
    sem_post(&resetDone);
}

void *read(void *arg){
    char c;
    while(1){
        sem_wait(&readThd);

        if(resetting){
            hitReset = true;
            sem_post(&inputCntThd);
            sem_post(&reset);
            sem_wait(&resetDone);
            //continue;
        }
        
        if(canAdd(inputBuf)){
            if((c = read_input()) == EOF){
                hitEOF = true;
                sem_post(&inputCntThd);
                break;
            }
            push(&inputBuf, c);
            sem_post(&inputCntThd);
        } else {
            fprintf(stderr, "ERROR: Entered read logic mistakenly\n");
        }
    }

    return NULL;
}

void *countInput(void *arg){
    bool hasWork;
    while(1){
        sem_wait(&inputCntThd);
        
        hasWork = canCount(inputBuf);
        if(hitEOF && !hasWork){
            inputCounted = true;
            sem_post(&encryptInThd);
            break;
        }

        if(hitReset && !hasWork){
            inputCntReset = true;
            sem_post(&reset);
            sem_post(&encryptInThd);
            sem_wait(&resetDone);
            continue;
        }

        if(hasWork){
            count_input(countNext(&inputBuf));
            sem_post(&encryptInThd);
        } else {
            fprintf(stderr, "ERROR: Entered input count logic mistakenly\n");
        }
    }
    return NULL;
}

void *encrypt_func(void *arg){
    bool hasWork;
    while(1){
        sem_wait(&encryptInThd);

        hasWork = canPop(inputBuf) || canCount(inputBuf);
        if(inputCounted && !hasWork){
            allEncrypted = true;
            sem_post(&outputCntThd);
            break;
        }

        if(inputCntReset && !hasWork){
            encryptReset = true;
            sem_post(&reset);
            sem_post(&outputCntThd);
            sem_wait(&resetDone);
            continue;
        }

        if(canPop(inputBuf)){
            sem_wait(&encryptOutThd);

            if(canAdd(outputBuf)){
                push(&outputBuf, encrypt(pop(&inputBuf)));
                sem_post(&outputCntThd);
                sem_post(&readThd);
            } else {
                fprintf(stderr, "ERROR: Entered encrypt add logic mistakenly\n");
            }
        } else {
            fprintf(stderr, "ERROR: Entered encrypt pop logic mistakenly\n");
        }
    }
    return NULL;
}

void *countOutput(void *arg){
    bool hasWork;
    while(1){
        sem_wait(&outputCntThd);

        hasWork = canCount(outputBuf) || canPop(inputBuf) || canCount(inputBuf);
        if(allEncrypted && !hasWork){
            outputCounted = true;
            sem_post(&writeThd);
            break;
        }
        if(encryptReset && !hasWork){
            outputCntReset = true;
            sem_post(&reset);
            sem_post(&writeThd);
            sem_wait(&resetDone);
            continue;
        }
        if(canCount(outputBuf)){
            count_output(countNext(&outputBuf));
            sem_post(&writeThd);
        } else {
            fprintf(stderr, "ERROR: Entered output count logic mistakenly\n");
        }

    }
    return NULL;
}

void *write(void *arg){
    bool hasWork;
    while(1){
        sem_wait(&writeThd);
        hasWork = canPop(outputBuf) || canCount(outputBuf) || canPop(inputBuf) || canCount(inputBuf);
        if(outputCounted && !hasWork){
            break;
        }
        if(outputCntReset && !hasWork){
            sem_post(&reset);
            sem_wait(&resetDone);
            continue;
        }
        if(canPop(outputBuf)){
            write_output(pop(&outputBuf));
            sem_post(&encryptOutThd);
        } else {
            fprintf(stderr, "ERROR: Entered write logic mistakenly\n");
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
	
    sem_init(&reset, 0, 0);
    sem_init(&resetDone, 0, 0);

    sem_init(&readThd, 0, inputBufSize);
    sem_init(&inputCntThd, 0, 0);
    sem_init(&encryptInThd, 0, 0);
    sem_init(&encryptOutThd, 0, outputBufSize);
    sem_init(&outputCntThd, 0, 0);
    sem_init(&writeThd, 0, 0);


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

    sem_destroy(&reset);
    sem_destroy(&resetDone);

    sem_destroy(&readThd);
    sem_destroy(&inputCntThd);
    sem_destroy(&encryptInThd);
    sem_destroy(&encryptOutThd);
    sem_destroy(&outputCntThd);
    sem_destroy(&writeThd);
}
