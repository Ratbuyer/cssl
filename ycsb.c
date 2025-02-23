#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "skiplist.h"
#include <time.h>

#define LOAD_SIZE 100000000
#define RUN_SIZE 100000000

// Operation types
enum {
    OP_INSERT,
    OP_UPDATE,
    OP_READ,
    OP_SCAN,
    OP_SCAN_END,
    OP_DELETE,
};

void ycsb_load_run_randint(const char *init_file, const char *txn_file,
                           uint64_t *init_keys, uint64_t *keys,
                           uint64_t *range_end, int *ranges, int *ops) {
    printf("Loading with file: %s\n", init_file);
    printf("Running with file: %s\n", txn_file);

    FILE *infile_load = fopen(init_file, "r");
    if (!infile_load) {
        perror("Failed to open load file");
        return;
    }

    char op[16];
    uint64_t key;
    size_t count = 0;
    while ((count < LOAD_SIZE) && fscanf(infile_load, "%s %lu", op, &key) == 2) {
        if (strcmp(op, "INSERT") != 0) {
            printf("READING LOAD FILE FAIL!\n");
            fclose(infile_load);
            return;
        }
        init_keys[count++] = key;
    }
    fclose(infile_load);
    printf("Loaded %lu keys\n", count);

    FILE *infile_txn = fopen(txn_file, "r");
    if (!infile_txn) {
        perror("Failed to open transaction file");
        return;
    }

    count = 0;
    int range;
    uint64_t rend;
    while ((count < RUN_SIZE) && fscanf(infile_txn, "%s %lu", op, &key) == 2) {
        if (strcmp(op, "INSERT") == 0) {
            ops[count] = OP_INSERT;
        } else if (strcmp(op, "UPDATE") == 0) {
            ops[count] = OP_UPDATE;
        } else if (strcmp(op, "READ") == 0) {
            ops[count] = OP_READ;
        } else if (strcmp(op, "SCAN") == 0) {
            fscanf(infile_txn, "%d", &range);
            ops[count] = OP_SCAN;
            ranges[count] = range;
        } else if (strcmp(op, "SCANEND") == 0) {
            fscanf(infile_txn, "%lu", &rend);
            ops[count] = OP_SCAN_END;
            range_end[count] = rend;
        } else {
            printf("UNRECOGNIZED CMD!\n");
            fclose(infile_txn);
            return;
        }
        keys[count++] = key;
    }
    fclose(infile_txn);
    printf("Loaded %lu more keys\n", count);
    
    printf("here\n");
    
    ////////////////// Load Phase //////////////////
    
    int counter = 0;
    
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);

    _CSSL_SkipList *slist = createSkipList(9, 5);
    for (size_t i = 0; i < LOAD_SIZE; i++) {
    	uint32_t key = (uint32_t)(init_keys[i] & 0xFFFFFFFF);
     	key = key / 1000;
      	// printf("Inserting key: %u\n", key);
        insertElement(slist, key);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end); 
    
    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_nsec - start.tv_nsec) / 1000;
    
    double throughput = (double)LOAD_SIZE / (elapsed_us);
    
    printf("Load throughput: %f ops/us\n", throughput);
    
    printf("Load complete.\n");

    for (size_t i = 0; i < RUN_SIZE; i++) {
    	uint32_t key = (uint32_t)(keys[i] & 0xFFFFFFFF);
     	key = key / 1000;
        if (ops[i] == OP_INSERT) {
            insertElement(slist, key);
        } else if (ops[i] == OP_READ) {
            searchElement(slist, key) == INT_MAX ? : counter++;
        }
    }

    printf("Run complete.\n");
    
    printf("Number of successful reads: %d\n", counter);
}

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s [index type] [loadfile] [index file] [output file]\n", argv[0]);
        return 1;
    }
    
    char load_file[256];
    char index_file[256];
    
    strncpy(load_file, argv[2], sizeof(load_file) - 1);
    load_file[sizeof(load_file) - 1] = '\0'; // Null terminate
    
    strncpy(index_file, argv[3], sizeof(index_file) - 1);
    index_file[sizeof(index_file) - 1] = '\0'; // Null terminate


    uint64_t *init_keys = (uint64_t *)malloc(LOAD_SIZE * sizeof(uint64_t));
    uint64_t *keys = (uint64_t *)malloc(RUN_SIZE * sizeof(uint64_t));
    uint64_t *range_end = (uint64_t *)malloc(RUN_SIZE * sizeof(uint64_t));
    int *ranges = (int *)malloc(RUN_SIZE * sizeof(int));
    int *ops = (int *)malloc(RUN_SIZE * sizeof(int));

    if (!init_keys || !keys || !range_end || !ranges || !ops) {
        perror("Memory allocation failed");
        return 1;
    }

    memset(init_keys, 0, LOAD_SIZE * sizeof(uint64_t));
    memset(keys, 0, RUN_SIZE * sizeof(uint64_t));
    memset(range_end, 0, RUN_SIZE * sizeof(uint64_t));
    memset(ranges, 0, RUN_SIZE * sizeof(int));
    memset(ops, 0, RUN_SIZE * sizeof(int));

    ycsb_load_run_randint(load_file, index_file, init_keys, keys, range_end, ranges, ops);

    free(init_keys);
    free(keys);
    free(range_end);
    free(ranges);
    free(ops);

    return 0;
}
