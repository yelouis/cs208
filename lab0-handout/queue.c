/*
 * Starter file for CS 208 Lab 0: Welcome to C
 * Adapted from lab developed at CMU by R. E. Bryant, 2017-2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if(q == NULL){
      printf("Memory allocation failed\n");
      return NULL;
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* How about freeing the list elements and the strings? */
    /* Free queue structure */
    if(q == NULL){
      return;
    }

    char *sp = NULL;
    size_t bufsize = 1;
    while(q -> size != 0){
      q_remove_head(q, sp, bufsize);
    }
    free(sp);
    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
  /* What should you do if the q is NULL? */
    if(q == NULL){
      printf("q is NULL\n");
      return false;
    }

    list_ele_t *newh;
    newh = malloc(sizeof(list_ele_t));
    if(newh == NULL){
      printf("Memory allocation failed\n");
      return false;
    }
    /* Don't forget to allocate space for the string and copy it */
    /* What if either call to malloc returns NULL? */
    int counter1 = 0;
    while(*(s+counter1) != 0x00){
      counter1 += 1;
    }
    counter1 += 1;

    newh -> value = malloc(sizeof(char)*counter1);
    if(newh -> value == NULL){
      printf("Memory allocation failed\n");
      return false;
    }
    int counter2 = 0;
    while(*(s+counter2) != 0x00){
      *((newh -> value) + counter2) = *(s+counter2);
      counter2 += 1;
    }
    *((newh -> value) + counter2) = 0x00;

    newh->next = q->head;
    q->head = newh;
    if(q->size == 0){
        q -> tail = newh;
    }
    q->size += 1;
    return true;
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    if(q == NULL){
      printf("q is NULL\n");
      return false;
    }

    if(q -> size == 0){
      return q_insert_head(q,s);
    }

    list_ele_t *newh;
    /* What should you do if the q is NULL? */
    newh = malloc(sizeof(list_ele_t));
    if(newh == NULL){
      printf("Memory allocation failed\n");
      return false;
    }
    /* Don't forget to allocate space for the string and copy it */
    /* What if either call to malloc returns NULL? */
    int counter = 0;
    while(*(s+counter) != 0x00){
      counter += 1;
    }
    counter += 1;

    newh -> value = malloc(sizeof(char)*counter);
    if(newh -> value == NULL){
      printf("Memory allocation failed\n");
      return false;
    }
    int counter2 = 0;
    while(*(s+counter2) != 0x00){
      *((newh -> value) + counter2) = *(s+counter2);
      counter2 += 1;
    }
    *((newh -> value) + counter2) = 0x00;

    newh->next = NULL;
    q->tail->next = newh;
    q->tail = newh;
    q->size += 1;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    /* You need to fix up this code. */
    if(q == NULL || q->size == 0){
      printf("q is NULL or empty\n");
      return false;
    }

    list_ele_t *deletingHead = q->head;
    q->head = q->head->next;
    q->size -= 1;

    //Copied to sp
    if(sp != NULL){
      int counter = 0;
      while(*((deletingHead -> value)+counter) != 0x00 && counter < bufsize-1){
        *(sp+counter) = *((deletingHead -> value)+counter);
        counter += 1;
      }
      *(sp+counter) = 0x00;
    }
    free(deletingHead->value);
    free(deletingHead);

    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if(q == NULL){
      printf("q is NULL\n");
      return 0;
    }else{
      return (q -> size);
    }
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
  if(q == NULL || q->size == 0){
    return;
  }

  list_ele_t *originalTail = q -> tail;
  list_ele_t *movingElement = q -> head;
  q -> tail = movingElement;
  while(movingElement != originalTail){
    q -> head = movingElement -> next;
    movingElement -> next = originalTail -> next;
    originalTail -> next = movingElement;
    movingElement = q -> head;
  }
    /* You need to write the code for this function */
}
