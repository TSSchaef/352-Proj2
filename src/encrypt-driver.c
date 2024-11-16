#include <stdio.h>
#include "encrypt-module.h"

void reset_requested() {
	log_counts();
}

void reset_finished() {
}

int main(int argc, char *argv[]) {
    if(argc != 4){
        fprintf(stderr, "ERROR: Must pass 3 files as commnad line arguments\n\"./encrypt in.txt out.txt log.txt\"\n");
        return -1;
    }  
	init(argv[1], argv[2], argv[3]); 
	
    char c;
	while ((c = read_input()) != EOF) { 
		count_input(c); 
		c = encrypt(c); 
		count_output(c); 
		write_output(c); 
	} 
	printf("End of file reached.\n"); 
	log_counts();
}
