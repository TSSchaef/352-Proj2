#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define MAX_COUNTS 256

// Function to extract counts from a line
int extract_count(const char *line, const char *key) {
    const char *pos = strstr(line, key);
    if (pos) {
        return atoi(pos + strlen(key));
    }
    return -1;
}

// Function to process a block
bool process_block(FILE *file) {
    char line[MAX_LINE_LENGTH];
    int inputCount = -1, outputCount = -1;

    while (fgets(line, sizeof(line), file)) {
        // Check if a new block starts
        if (strstr(line, "Counts using key")) {
            fseek(file, -strlen(line), SEEK_CUR); // Rewind to allow next block processing
            break;
        }

        if (strstr(line, "Total input count")) {
            inputCount = extract_count(line, "Total input count: ");
        } else if (strstr(line, "Total output count")) {
            outputCount = extract_count(line, "Total output count: ");
        }
    }

    if (inputCount == -1 || outputCount == -1) {
        fprintf(stderr, "Error: Failed to parse block.\n");
        return false;
    }

    return inputCount == outputCount;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    bool allTestsPassed = true;
    while (!feof(file)) {
        char line[MAX_LINE_LENGTH];
        if (fgets(line, sizeof(line), file) && strstr(line, "Counts using key")) {
            // Process each block
            bool result = process_block(file);
            if (!result) {
                printf("Test failed\n");
                allTestsPassed = false;
                break;
            }
        }
    }

    if (allTestsPassed) {
        printf("Test passed\n");
    }

    fclose(file);
    return EXIT_SUCCESS;
}

