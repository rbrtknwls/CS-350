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

typedef struct { struct Article * art; char * word; int sum; } input;


void *CountOccur(void *arg) {
    input *args = (input *) arg;

    size_t wordCount = 0;
    for ( unsigned int j = 0; j < args->art->numWords; j++)
    {
        // Get the length of the function.
        size_t len = strnlen( args->art->words[j], MAXWORDSIZE );
        if ( !strncmp( args->art->words[j], args->word, len ) )
        {
            wordCount += 1;
        }
    }

    args->sum = wordCount;

    pthread_exit(NULL);
}

size_t MultithreadedWordCount( struct  Library * lib, char * word)
{
  printf("Parallelizing with %d threads...\n",NUMTHREADS);
    /* XXX FILLMEIN
     * Also feel free to remove the printf statement
     * to improve time */
  pthread_t threads[lib->numArticles];

  input argu[lib->numArticles];

  for (int i = 0; i < lib->numArticles; i++) {
      argu[i].art = lib->articles[i];
      argu[i].word = word;
  }

  for (int i = 0; i < lib->numArticles; i++) {
      pthread_create(threads[i], NULL, CountOccur, &argu[i]);
  }

  int sum = 0;
  for (int i = 0; i < lib->numArticles; i++) {
      pthread_join(threads[i], NULL);

      sum += argu[i].sum;
  }

  return sum;
}
