/*
 * Simple allocator based on implicit free lists, first fit search,
 * and boundary tag coalescing.
 *
 * Each block has header and footer of the form:
 *
 *      64                  4  3  2  1  0
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  0  0  0  a/f
 *      -----------------------------------
 *
 * where s are the meaningful size bits and a/f is 1
 * if and only if the block is allocated. The list has the following form:
 *
 * begin                                                             end
 * heap                                                             heap
 *  -----------------------------------------------------------------
 * |  pad   | hdr(16:a) | ftr(16:a) | zero or more usr blks | hdr(0:a) |
 *  -----------------------------------------------------------------
 *          |       prologue        |                       | epilogue |
 *          |         block         |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Yes",
    /* First member's full name */
    "Yuting Su",
    /* First member's email address */
    "suy@carleton.edu",
    /* Second member's full name (leave blank if none) */
    "Louis Ye",
    /* Second member's email address (leave blank if none) */
    "yel@carleton.edu"
};

/* Basic constants and macros */
#define WSIZE       8       /* word size (bytes) */
#define DSIZE       16      /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* initial heap size (bytes) */
#define OVERHEAD    16      /* overhead of header and footer (bytes) */

/* NOTE: feel free to replace these macros with helper functions and/or
 * add new ones that will be useful for you. Just make sure you think
 * carefully about why these work the way they do
 */

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(size_t *)(p))
#define PUT(p, val)  (*(size_t *)(p) = (val))

/* Perform unscaled pointer arithmetic */
#define PADD(p, val) ((char *)(p) + (val))
#define PSUB(p, val) ((char *)(p) - (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0xf)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       (PSUB(bp, WSIZE))
#define FTRP(bp)       (PADD(bp, GET_SIZE(HDRP(bp)) - DSIZE))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  (PADD(bp, GET_SIZE(HDRP(bp))))
#define PREV_BLKP(bp)  (PSUB(bp, GET_SIZE((PSUB(bp, DSIZE)))))

/* Global variables */

// Pointer to first block
static void *heap_start = NULL;

/* Function prototypes for internal helper routines */

static bool check_heap(int lineno);
static void print_heap();
static void print_block(void *bp);
static bool check_block(int lineno, void *bp);
static void *extend_heap(size_t size);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void place(void *bp, size_t asize);
static size_t max(size_t x, size_t y);

/*
 * mm_init -- <What does this function do?>
 * <What are the function's arguments?>
 * <What is the function's return value?>
 * <Are there any preconditions or postconditions?>
 */
int mm_init(void) {
    /* create the initial empty heap */
    if ((heap_start = mem_sbrk(4 * WSIZE)) == NULL)
        return -1;

    PUT(heap_start, 0);                        /* alignment padding */
    PUT(PADD(heap_start, WSIZE), PACK(OVERHEAD, 1));  /* prologue header */
    PUT(PADD(heap_start, DSIZE), PACK(OVERHEAD, 1));  /* prologue footer */
    PUT(PADD(heap_start, WSIZE + DSIZE), PACK(0, 1));   /* epilogue header */

    heap_start = PADD(heap_start, DSIZE); /* start the heap at the (size 0) payload of the prologue block */

    printf("get to init\n");

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    print_heap();
    check_heap(__LINE__);
    return 0;
}

/*
 * mm_malloc -- <What does this function do?>
 * <What are the function's arguments?>
 * <What is the function's return value?>
 * <Are there any preconditions or postconditions?>
 */
void *mm_malloc(size_t size) {
    size_t asize;      /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;
    check_heap(__LINE__);

    /* Ignore spurious requests */
    if (size <= 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE) {
        asize = DSIZE + OVERHEAD;
    } else {
        /* Add overhead and then round up to nearest multiple of double-word alignment */
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE - 1)) / DSIZE);
    }

    check_heap(__LINE__);
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    check_heap(__LINE__);
    /* No fit found. Get more memory and place the block */
    extendsize = max(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;

    check_heap(__LINE__);
    place(bp, asize);
    check_heap(__LINE__);
    return bp;
}

/*
 * mm_free -- Free a block given a pointer to the block
 * A pointer (block pointer)
 * Should we return a pointer to the block we freed or should we return a 1 or 0
 * indicating that the block has been freed. WE DON'T RETURN ANYTHING CAUSE IT IS
 * VOID.
 * Precondition: We can only free alocated blocks.
 * Precondition: Only free blocks within our heap. Don't run past the prologue and epilogue
 * Postcondition: Valid bit becomes 0.
 */
void mm_free(void *bp) {

    char *curHdr = HDRP(bp);
    char *curFtr = FTRP(bp);

    //check if the current block is allocated
    if(GET_ALLOC(curHdr) == 0x0){
        printf("The block is already free!");
        return;
    }

    size_t blockSize = GET_SIZE(curHdr);

    check_heap(__LINE__);
    PUT(curHdr, PACK(blockSize, 0x0));
    PUT(curFtr, PACK(blockSize, 0x0));

    check_heap(__LINE__);
    coalesce(bp);
    check_heap(__LINE__);


    // If GET_ALLOC of that pointer is 0, we can just return and print a message out
    // to indicate that the block was already freed.


    // GET_SIZE(p) << 1 and then use PUT to put the result of that into the address
    // to change the header
    // PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 0x0))
    // This will depend on whether GET_SIZE and GET_ALLOC returns something in 8 bytes
    // or not.

    // Use get size to get to footer so that we can set the footer valid tag to 0.
    // PUT(FTRP(bp), GET_SIZE(bp) << 1)
    // PUT(FTRP(bp), PACK(GET_SIZE(bp), 0b0))


    // Coalesce!!!!!

}

/*
 * EXTRA CREDIT
 * mm_realloc -- <What does this function do?>
 * <What are the function's arguments?>
 * <What is the function's return value?>
 * <Are there any preconditions or postconditions?>
*/
void *mm_realloc(void *ptr, size_t size) {
    // TODO: implement this function for EXTRA CREDIT
    return NULL;
}


/* The remaining routines are internal helper routines */


/*
 * place -- Place block of asize bytes at start of free block bp
 *          and <How are you handling splitting?>
 * Takes a pointer to a free block and the size of block to place inside it
 * Returns nothing
 * Precondition: We know that there is room in the list because both times place is called,
 * in malloc, we know that there is room avaliable. Called after find_fit and extend_heap.
 # Precondition: asize is a factor of 16.
 * Postcondition: Placed block has an allocated tag
 */
static void place(void *bp, size_t asize) {

    size_t curSize = GET_SIZE(HDRP(bp));

    if ((curSize - asize) >= (DSIZE + OVERHEAD)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        char *nextBp = NEXT_BLKP(bp);
        PUT(HDRP(nextBp), PACK(curSize-asize, 0));
        PUT(FTRP(nextBp), PACK(curSize-asize, 0));
    }else {
        PUT(HDRP(bp), PACK(curSize, 1));
        PUT(FTRP(bp), PACK(curSize, 1));
    }


    // char *curHdr = HDRP(bp);
    // char *curFtr = FTRP(bp);
    // size_t totalFreeSize = GET_SIZE(curHdr);
    //
    // //check if slitting is necessary
    // if (totalFreeSize == asize){
    //     check_heap(__LINE__);
    //     PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
    //     PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
    //     check_heap(__LINE__);
    //     return;
    // }
    //
    // size_t leftOverSize = totalFreeSize - asize;
    //
    // //header and footer of asize block
    // check_heap(__LINE__);
    // PUT(curHdr, PACK(asize - 16, 1));
    // PUT(PADD(curHdr, asize - 8), PACK(asize-16,1));
    //
    // //header and footer of leftover space
    // PUT(PADD(curHdr, asize), PACK(leftOverSize, 0));
    // PUT(curFtr, PACK(leftOverSize, 0));
    // check_heap(__LINE__);
    // return;

    // freeSize = GET_SIZE(HDRP(bp))
    // leftoverSpace = freeSize - asize
    // PUT(HDRP(bp), PACK(asize-16, 1))               // Set the header to asize without OVERHEAD
    // PUT((HDRP(bp) + asize - 8), PACK(asize-16,1))  // Set the footer to asize without OVERHEAD. Have to manually find footer
    // PUT((HDRP(bp)+asize), PACK(asize-16, 0))       // Set the header of the leftoverSpace
    // PUT(FTRP(bp), PACK(leftoverSpace, 0))          // Set the footer of the leftoverSpace

    // REPLACE THIS
    // currently does no splitting, just allocates the entire free block

}

/*
 * coalesce -- Boundary tag coalescing.
 * Takes a pointer to a free block
 * Return ptr to coalesced block
 * <Are there any preconditions or postconditions?>
 */
static void *coalesce(void *bp) {
    size_t prevBlock = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    printf("Printing heap now\n");
    print_heap();
    size_t nextBlock = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(!prevBlock && !nextBlock){
      return bp;
    }else if(prevBlock && !nextBlock){
      // Coalesce with next block
      size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size,0));
      return(bp);
    }else if(!prevBlock && nextBlock){
      // Coalesce with prev block
      size += GET_SIZE(HDRP(PREV_BLKP(bp)));
      PUT(FTRP(bp), PACK(size, 0));
      PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
      return(PREV_BLKP(bp));
    }else{
      // Coalesce with both
      size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
      PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
      PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
      return(PREV_BLKP(bp));
    }

    // if(prevBlock == 0x0 && nextBlock == 0x0){
    //     size_t sizeCur = GET_SIZE(PREV_BLKP(bp));
    //     size_t sizePre = GET_SIZE(bp);
    //     size_t sizePost = GET_SIZE(NEXT_BLKP(bp));
    //     size_t totalSize = sizeCur + sizePre +sizePost + 32;
    //
    //     //coalesce with pre
    //     check_heap(__LINE__);
    //     PUT(HDRP(PREV_BLKP(bp)), PACK(totalSize, 0));
    //     PUT(FTRP(NEXT_BLKP(bp)), PACK(totalSize, 0));
    //     check_heap(__LINE__);
    //     return PREV_BLKP(bp);
    // }
    // else if(prevBlock == 0x0){
    //     size_t sizeCur = GET_SIZE(PREV_BLKP(bp));
    //     size_t sizePre = GET_SIZE(bp);
    //     size_t totalSize = sizeCur + sizePre + 16;
    //
    //     check_heap(__LINE__);
    //     PUT(HDRP(PREV_BLKP(bp)), PACK(totalSize, 0));
    //     PUT(FTRP(bp), PACK(totalSize, 0));
    //     check_heap(__LINE__);
    //     return PREV_BLKP(bp);
    // }
    // else if(nextBlock == 0x0){
    //     size_t sizeCur = GET_SIZE(PREV_BLKP(bp));
    //     size_t sizePost = GET_SIZE(NEXT_BLKP(bp));
    //     size_t totalSize = sizeCur +sizePost + 16;
    //
    //     check_heap(__LINE__);
    //     PUT(HDRP(bp), PACK(totalSize, 0));
    //     PUT(FTRP(NEXT_BLKP(bp)), PACK(totalSize, 0));
    //     check_heap(__LINE__);
    //     return bp;
    // }



    // Going to have helper functions static void *coalescePrev(void *current, void *prev)
    // Helper function 2: static void *coalesceNext(void *current, void *next)

    // coalescePrev:
    // If both current and prev are not allocated.
    // !GET_ALLOC(PREV_BLKP(bp)) & !GET_ALLOC(bp):
    // size1 = GET_SIZE(PREV_BLKP(bp))
    // size2 = GET_SIZE(bp)
    // totalSize = size1 + size2 + 16
    // PUT(HDRP(PREV_BLKP(bp)), PACK(totalSize, 0))
    // PUT(FTRP(bp), PACK(totalSize, 0))

}


/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize) {
    /* search from the start of the free list to the end */
    for (char *cur_block = heap_start; GET_SIZE(HDRP(cur_block)) > 0; cur_block = NEXT_BLKP(cur_block)) {
        if (!GET_ALLOC(HDRP(cur_block)) && (asize <= GET_SIZE(HDRP(cur_block))))
            return cur_block;
    }

    return NULL;  /* no fit found */
}

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = words * WSIZE;
    if (words % 2 == 1)
        size += WSIZE;
    // printf("extending heap to %zu bytes\n", mem_heapsize());
    if ((long)(bp = mem_sbrk(size)) < 0)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * check_heap -- Performs basic heap consistency checks for an implicit free list allocator
 * and prints out all blocks in the heap in memory order.
 * Checks include proper prologue and epilogue, alignment, and matching header and footer
 * Takes a line number (to give the output an identifying tag).
 */
static bool check_heap(int line) {
    char *bp;

    if ((GET_SIZE(HDRP(heap_start)) != DSIZE) || !GET_ALLOC(HDRP(heap_start))) {
        printf("(check_heap at line %d) Error: bad prologue header\n", line);
        return false;
    }

    for (bp = heap_start; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!check_block(line, bp)) {
            return false;
        }
    }

    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
        printf("(check_heap at line %d) Error: bad epilogue header\n", line);
        return false;
    }

    return true;
}

/*
 * check_block -- Checks a block for alignment and matching header and footer
 */
static bool check_block(int line, void *bp) {
    if ((size_t)bp % DSIZE) {
        printf("(check_heap at line %d) Error: %p is not double-word aligned\n", line, bp);
        return false;
    }
    if (GET(HDRP(bp)) != GET(FTRP(bp))) {
        printf("(check_heap at line %d) Error: header does not match footer\n", line);
        return false;
    }
    return true;
}

/*
 * print_heap -- Prints out the current state of the implicit free list
 */
static void print_heap() {
    char *bp;

    printf("Heap (%p):\n", heap_start);

    for (bp = heap_start; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        print_block(bp);
    }

    print_block(bp);
}

/*
 * print_block -- Prints out the current state of a block
 */
static void print_block(void *bp) {
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: End of free list\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp,
       hsize, (halloc ? 'a' : 'f'),
       fsize, (falloc ? 'a' : 'f'));
}

/*
 * max: returns x if x > y, and y otherwise.
 */
static size_t max(size_t x, size_t y) {
    return (x > y) ? x : y;
}
