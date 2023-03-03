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

typedef struct { struct Article * art; char * word; } input;
typedef struct { int a; } output;


void *CountOccur(void *arg) {
    input *args = (input *) arg;
    output *rvals = malloc(sizeof(output));

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

    printf("Num words is: %ld \n", wordCount);

    return rvals;
}

size_t MultithreadedWordCount( struct  Library * lib, char * word)
{
  printf("Parallelizing with %d threads...\n",NUMTHREADS);
    /* XXX FILLMEIN
     * Also feel free to remove the printf statement
     * to improve time */
  pthread_t p;
  volatile output *ret = {0};

  input argu[lib->numArticles];

  for (int i = 0; i < lib->numArticles; i++) {
      argu[i]->art = lib->articles[i];
      argu[i]->word = word;
  }

  for (int i = 0; i < lib->numArticles; i++) {
      pthread_create(&p, NULL, CountOccur, &argu[i]);
  }
  for (int i = 0; i < lib->numArticles; i++) {
      pthread_join(p, (void **) &ret);
  }

  return 0;
}
