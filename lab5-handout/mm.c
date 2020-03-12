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



 /*
 *  Structure for explicit.
 *  Free block
 *  [ HEADER | PREV | NEXT |    PAYLOAD    | FOOTER ]
 *  Allocated block
 *  [ HEADER |   PAYLOAD    | FOOTER ]
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
#define OVERHEADEX  32      /* overhead of header and footer (bytes) of explicit */


/* NOTE: feel free to replace these macros with helper functions and/or
 * add new ones that will be useful for you. Just make sure you think
 * carefully about why these work the way they do
 */

#define ALIGN(p) (((size_t)(p) + (16-1)) & ~0x7)


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

/* Get the next free block given pointer */
#define PREV_FREE_BLKP(bp)  (*(void **)(GET_P(bp)))
#define NEXT_FREE_BLKP(bp)  (*(void **) (GET_P((PADD(bp, WSIZE)))))

#define GET_P(p) ((void **)(p))
#define PUT_P(p, val)  (*(void **)(p) = (val))

/* Global variables */

// Pointer to first block
static void *heap_start = NULL;
static void *free_listp = NULL;

/* Function prototypes for internal helper routines */

static bool check_heap(int lineno);
static void print_heap();
static void print_block(void *bp);
static bool check_block(int lineno, void *bp);
static void *extend_heap(size_t size);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void rmv_from_free(void *bp);
static void insert_front(void *bp);
static void place(void *bp, size_t asize);
static size_t max(size_t x, size_t y);
static void print_free_heap();

//Currently a version that works up till trace 6

/*
 * mm_init -- <What does this function do?>
 * <What are the function's arguments?>
 * <What is the function's return value?>
 * <Are there any preconditions or postconditions?>
 */
int mm_init(void) {
  free_listp = NULL;
    /* create the initial empty heap */
    if ((heap_start = mem_sbrk(4 * WSIZE)) == NULL)
        return -1;

    //Making a free_list to keep track of all the free blocks
    //free_listp = heap_start;

    PUT(heap_start, 0);                        /* alignment padding */
    PUT(PADD(heap_start, 1*WSIZE), PACK(OVERHEAD, 1));  /* prologue header */
    PUT(PADD(heap_start, 2*WSIZE), PACK(OVERHEAD, 1));  /* prologue footer */
    PUT(PADD(heap_start, 3*WSIZE), PACK(0, 1));   /* epilogue header */

    heap_start = PADD(heap_start, DSIZE); /* start the heap at the (size 0) payload of the prologue block */

     // printf("heap in init\n");
     // print_heap();
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    // PUT(PADD(heap_start, DSIZE), PACK(0, 0));
    // PUT(PADD(heap_start, DSIZE+WSIZE), PACK(0, 0));

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
    // printf("malloc\n");

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
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        // printf("enough space\n");
        return bp;
    }
    /* No fit found. Get more memory and place the block */
    // printf("need to get more memory for heap\n");
    extendsize = max(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;

    place(bp, asize);
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
    // printf("Going to free a block\n");
    // print_heap();
    char *curHdr = HDRP(bp);
    char *curFtr = FTRP(bp);

    //check if the current block is allocated
    if(GET_ALLOC(curHdr) == 0){
        printf("The block is already free!");
        return;
    }

    size_t blockSize = GET_SIZE(curHdr);

    PUT(curHdr, PACK(blockSize, 0));
    PUT(curFtr, PACK(blockSize, 0));

    // printf("Freed a block\n");
    // print_heap();
    coalesce(bp);
}

/*
 * EXTRA CREDIT
 * mm_realloc -- <What does this function do?>
 * <What are the function's arguments?>
 * <What is the function's return value?>
 * <Are there any preconditions or postconditions?>
*/
void *mm_realloc(void *ptr, size_t size) {
  size_t oldsize, asize;
  void *newptr;

  asize = max(ALIGN(size) + DSIZE, 32);
  /* If size == 0 then this is just free, and we return NULL. */
  if(size == 0) {
      free(ptr);
      return 0;
  }

  /* If oldptr is NULL, then this is just malloc. */
  if(ptr == NULL) {
      return malloc(size);
  }

  /* Original block size */
  oldsize = GET_SIZE(HDRP(ptr));

  /* If new size is same as old, just return */
  if (asize == oldsize)  return ptr;

  newptr = mm_malloc(size);

  /* If realloc() fails the original block is left untouched  */
  if(!newptr) {
      return 0;
  }

  /* Copy the old data. */
  if(size < oldsize) oldsize = size;
  memcpy(newptr, ptr, oldsize);

  /* Free the old block. */
  free(ptr);

  return newptr;
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
    // printf("top of place\n");
    // print_heap();

  if ((curSize - asize) >= DSIZE+OVERHEAD) {
        // printf("before place, splitting\n");
        // print_heap();
        rmv_from_free(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        char *nextBp = NEXT_BLKP(bp);
        PUT(HDRP(nextBp), PACK(curSize-asize, 0));
        PUT(FTRP(nextBp), PACK(curSize-asize, 0));
        // printf("after place, splitting\n");
        // print_heap();
        coalesce(nextBp);
  }
  /* not enough space for free block, don't split */
  else {
        // printf("before place, no splitting\n");
        // print_heap();
        PUT(HDRP(bp), PACK(curSize, 1));
        PUT(FTRP(bp), PACK(curSize, 1));
        // printf("after place, no splitting\n");
        // print_heap();
        rmv_from_free(bp);
  }

}



    // size_t curSize = GET_SIZE(HDRP(bp));
    //
    // // Uses part of the block
    // if ((curSize - asize) >= DSIZE) {
    //     PUT(HDRP(bp), PACK(asize, 1));
    //     PUT(FTRP(bp), PACK(asize, 1));
    //     char *nextBp = NEXT_BLKP(bp);
    //     PUT(HDRP(nextBp), PACK(curSize-asize, 0));
    //     PUT(FTRP(nextBp), PACK(curSize-asize, 0));
    // }else {
    //   // Uses entire block
    //     PUT(HDRP(bp), PACK(curSize, 1));
    //     PUT(FTRP(bp), PACK(curSize, 1));
    // }



/*
 * coalesce -- Boundary tag coalescing.
 * Takes a pointer to a free block
 * Return ptr to coalesced block
 * <Are there any preconditions or postconditions?>
 */
 static void *coalesce(void *bp)
 {
     /* get tags of next and previous blocks */
 	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
 	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

 	size_t size = GET_SIZE(HDRP(bp));
  // printf("Before coalesce\n");
  // print_heap();

     /* case 2 */
 	if (prev_alloc && !next_alloc)
 	{
    // printf("Coalesce with next\n");
 		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));  /* add size of next free block */
 		rmv_from_free(NEXT_BLKP(bp));           /* remove the block from free list */
 		PUT(HDRP(bp), PACK(size, 0));
 		PUT(FTRP(bp), PACK(size, 0));
 	}

     /* case 3 */
 	else if (!prev_alloc && next_alloc)
 	{
    // printf("Coalesce with prev\n");
 	  size += GET_SIZE(HDRP(PREV_BLKP(bp)));    /* add size of previous free block */
 	  bp = PREV_BLKP(bp);
 	  rmv_from_free(bp);                         /* remove the block from free list */
 	  PUT(HDRP(bp), PACK(size, 0));
 	  PUT(FTRP(bp), PACK(size, 0));
 	}

     /* case 4 */
 	else if (!prev_alloc && !next_alloc)
 	{
    // printf("Coalesce with both\n");
         /* add size of next and previous free block */
 		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
 		rmv_from_free(PREV_BLKP(bp));   /* remove the block from free list */
 		rmv_from_free(NEXT_BLKP(bp));   /* remove the block from free list */
 		bp = PREV_BLKP(bp);
 		PUT(HDRP(bp), PACK(size, 0));
 		PUT(FTRP(bp), PACK(size, 0));
 	}else if(prev_alloc && next_alloc){
    // printf("Coalesce with nothing\n");
  }

     /* if case 1 occurs, it will drop down here without merging with any blocks */
 	insert_front(bp);
  // print_heap();
 	return bp;

    // if(prevBlock && nextBlock){
    //
    //   return bp;
    // }else if(prevBlock && !nextBlock){
    //   // Coalesce with next block
    //   size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    //   PUT(HDRP(bp), PACK(size, 0));
    //   PUT(FTRP(bp), PACK(size,0));
    //
    //   return(bp);
    // }else if(!prevBlock && nextBlock){
    //   // Coalesce with prev block
    //   size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    //   PUT(FTRP(bp), PACK(size, 0));
    //   PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    //
    //   return(PREV_BLKP(bp));
    // }else{
    //   // Coalesce with both
    //   size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    //   PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    //   PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    //
    //   return(PREV_BLKP(bp));
    // }

}

static void insert_front(void *bp)
{
  // printf("Inserting into free list\n");
  // print_free_heap();

    if(free_listp == NULL){
      // printf("Inserting into empty free list\n");

      PUT_P(bp, NULL);
      PUT_P(PADD(bp, 8), NULL);
      free_listp = bp;
    }else{
      // printf("Inserting into free list with something\n");

      PUT_P(bp, NULL);
      PUT_P(PADD(bp, 8), free_listp);
      PUT_P(free_listp, bp);
      free_listp = bp;
    }
    // print_free_heap();
	return;
}

static void rmv_from_free(void *bp)
{
  // printf("removing from free list\n");

  // print_free_heap();
    if (PREV_FREE_BLKP(bp) != NULL && NEXT_FREE_BLKP(bp) != NULL){
        // printf("many things in free list\n");

        PUT_P(NEXT_FREE_BLKP(bp), PREV_FREE_BLKP(bp));
        PUT_P(PADD(PREV_FREE_BLKP(bp), 8), NEXT_FREE_BLKP(bp));
    }else if (PREV_FREE_BLKP(bp) == NULL && NEXT_FREE_BLKP(bp) != NULL){
        // printf("first thing in free list\n");

        PUT_P(NEXT_FREE_BLKP(bp), NULL);
        free_listp = NEXT_FREE_BLKP(bp);
    }else if(PREV_FREE_BLKP(bp) != NULL && NEXT_FREE_BLKP(bp) == NULL){
        // printf("last thing in free list\n");

        PUT_P(PADD(PREV_FREE_BLKP(bp), 8), NULL);
    }else{
        // printf("only thing in free list\n");
        free_listp = NULL;
    }
    // print_free_heap();

    return;
}

static void *find_fit(size_t asize)
{
    void *bp;

    /* traverse free list */
    for (bp = free_listp; bp != NULL; bp = NEXT_FREE_BLKP(bp)) {
      // printf("Looping\n");
        if (asize <= (size_t)GET_SIZE(HDRP(bp)))
	        return bp;
    }

    // printf("Returned NULL in find_fit\n");

    return NULL; // No fit
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
    // printf("before extending\n");
    // print_heap();
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(PADD(HDRP(bp), WSIZE), 0);
    PUT(PADD(HDRP(bp), DSIZE), 0);
    //PUT(PREV_FREE_BLKP(bp), 0);
    //PUT(NEXT_FREE_BLKP(bp), 0);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */
    // printf("after extending\n");
    // print_heap();

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
 * print_heap -- Prints out the current state of the implicit free list
 */
static void print_free_heap() {
    char *bp;

    printf("Heap (%p):\n", free_listp);

    // bp = free_listp;
    //
    // if(bp == NULL){
    //   printf("Empty free list\n");
    //   return;
    // }
    //
    // while(GET_P(bp) != NULL){
    //   print_block(GET_P(bp));
    //   bp = NEXT_FREE_BLKP(bp);
    // }

    for (bp = free_listp; bp != NULL; bp = NEXT_FREE_BLKP(bp)) {
        print_block(bp);
    }

    //print_block(bp);
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
