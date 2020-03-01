#include "cachelab.h"

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int bigE = 0; /* associativity */
char* traceFile = NULL;

int bigS;
int bigB;

int missCount = 0;
int hitCount = 0;
int evictionCount = 0;

/* Memory address
 * Without using pointers cause we want to simulate addresses and not actual addresses?
 * At least that is what I have been told
 */
typedef unsigned long long int memAddr;

typedef struct cacheLine {
    char valid;
    memAddr tag;
    int counter;
    struct cacheLine * next;
} cacheLineA;

typedef cacheLineA* cacheSetA;
typedef cacheSetA* cacheA;

cacheA cache;





void accessData(memAddr addr)
{
	int minLRU = INT_MAX; 	//min counter in the set
	int maxLRU = 0;		//max counter in the set

	// Get the address of the first s bit, and the set number of addr
	memAddr sBit = addr >> b;
	memAddr set = sBit & ((int)pow(2,s) - 1);

	// Get the address of the first t bit, and the tag of addr
	memAddr tBit = addr >> (b + s);
	memAddr tag = tBit & ((int)pow(2, 64 - (b+s)) -1);

	// Search the set for the largest counter value
	for (int i = 0; i < bigE; i++) {
		if ( (*(cache + set) + i)->counter > maxLRU && (*(cache + set) + i)->valid == 1) {
			maxLRU = (*(cache + set) + i)->counter;
		}
	}

	// If there is a hit, update hitCount and most recently used line
	for (int i = 0; i < bigE; i++) {
		if ( (*(cache + set) + i)->tag == tag && (*(cache + set) + i)->valid == 1) {
			(*(cache + set) + i)->counter = maxLRU + 1;
			hitCount++;
			return;
		}
	}

	// If there is a miss, update missCount
	for (int i = 0; i < bigE; i++) {
		if ( (*(cache + set) + i)->valid == 0) {
			(*(cache + set) + i)->tag = tag;
			(*(cache + set) + i)->valid = 1;
			(*(cache + set) + i)->counter = maxLRU + 1;
			missCount++;
			return;
		}
	}

	// Find the LRU line
	// Then replace that line
	for (int i = 0; i < bigE; i++) { //find the LRU line
		if ( (*(cache + set) + i)->counter < minLRU && (*(cache + set) + i)->valid == 1) {
			minLRU = (*(cache + set) + i)->counter;
		}
	}
	for (int i = 0; i < bigE; i++) { //replace the line
		if ( (*(cache + set) + i)->counter == minLRU) {
			(*(cache + set) + i)->tag = tag;
			(*(cache + set) + i)->valid = 1;
			(*(cache + set) + i)->counter = maxLRU + 1;
			missCount++;
			evictionCount++;
			return;
		}
	}
}

void replayTrace(char* trace_fn)
{
    char buf[1000];
    memAddr addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while( fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
        	sscanf(buf+3, "%llx,%u", &addr, &len);

        	if(verbosity)
                	printf("%c %llx,%u ", buf[1], addr, len);

		accessData(addr);

		if (buf[1] == 'M')
			accessData(addr);

        	if (verbosity)
                	printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

void printSummary(int hits, int misses, int switches)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, switches);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, switches);
    fclose(output_fp);
}

int main(int argc, char* argv[])
{
    char c;

    // Parse the command line arguments: -h, -v, -s, -E, -b, -t
    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            bigE = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            traceFile = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    if (s == 0 || bigE == 0 || b == 0 || traceFile == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    // Compute bigS = 2^s and bigB = 2^b
    bigS = ( 1 << s);
    bigB = ( 1 << b);

    // Initialize the cache
    cache = malloc(bigS * sizeof(cacheSetA));		//allocate space for bigS number of sets
    for (int i = 0; i < bigS; i++) {

      cache[i] = malloc(bigE * sizeof(cacheLineA));	//allocate space for bigE number of lines in each set
      for (int j = 0; j < bigE; j++) {
        cache[i][j].valid = 0;
        cache[i][j].tag = 0;
        cache[i][j].counter = 0;
        cache[i][j].next = NULL;

        //set next of previous line to current line
        if (i > 0 && j==0) { //if it is the beginning of a new set and there is a previous set
          cache[i-1][bigE-1].next = &cache[i][j];
        }
        else if (j>0) { //as long as not at the first line at index i=0 j=0, set next of previous line
          cache[i][j-1].next = &cache[i][j];
        }
      }
    }

    replayTrace(traceFile);

    /* Free memory */
    for (int i = 0; i < bigE; i++) {
  		free(*(cache + i));
  	}
  	free(cache);
  	cache = NULL;

    printSummary(hitCount, missCount, evictionCount);
    return 0;
}
