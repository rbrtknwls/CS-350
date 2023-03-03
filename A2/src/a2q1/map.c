/* map.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic
 *  synchronization.
 *
 *  YOU MAY ADD WHATEVER YOU LIKE TO THIS FILE.
 *  YOU CANNOT CHANGE THE SIGNATURE OF MultithreadedWordCount.
 * ----------------------------------------------------------
 */
#include "data.h"

#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------
 * MultithreadedWordCount
 * --------------------------------------------------------------------
 * Takes a Library of articles containing words and a word.
 * Returns the total number of times that the word appears in the
 * Library.
 *
 * For example, "There the thesis sits on the theatre bench.", contains
 * 2 occurences of the word "the".
 * --------------------------------------------------------------------
 */

typedef struct { Article * art; char * word; } input;
typedef struct { int a } output;


void *CountOccur(void *arg) {
    input *args = (input *) arg;
    output *rvals = Malloc(sizeof(output));

    printf("HI from %d \n, word is %s", *x, args->word);

    return output;
}

size_t MultithreadedWordCount( struct  Library * lib, char * word)
{
  printf("Parallelizing with %d threads...\n",NUMTHREADS);
    /* XXX FILLMEIN
     * Also feel free to remove the printf statement
     * to improve time */
  pthread_t p;
  volatile output *ret = {0};

  for (int i = 0; i < lib->numArticles; i++) {
      input args = {lib->articles[i], word};
      pthread_create(&p, NULL, CountOccur, &args);
  }
  for (int i = 0; i < lib->numArticles; i++) {
      pthread_join(p, (void **) &ret);
  }

  return 0;
}
