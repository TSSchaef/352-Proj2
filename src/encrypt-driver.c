#include <stdio.h>
#include <pthread.h>
#include "encrypt-module.h"
#include "circular-buffer.h"

int inputBufSize = -1, outputBufSize = -1;
circular_buffer inputBuf, outputBuf;

void reset_requested() {
	log_counts();
}

void reset_finished() {
}

void *read(void *arg){
    return NULL;
}
void *countInput(void *arg){
    return NULL;
}
void *encrypt_func(void *arg){
    return NULL;
}
void *countOutput(void *arg){
    return NULL;
}
void *write(void *arg){
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 4){
        fprintf(stderr, "ERROR: Must pass 3 files as commnad line arguments\n\"./encrypt in.txt out.txt log.txt\"\n");
        return -1;
    }  

	init(argv[1], argv[2], argv[3]); 

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

    init_buffer(&inputBuf, inputBufSize);
    init_buffer(&outputBuf, outputBufSize);
	
    
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
}
