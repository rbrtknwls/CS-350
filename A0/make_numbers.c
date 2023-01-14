// Program for generting random integers between lo and hi

#include <stdio.h>
#include <stdlib.h>

int main (void) {
   int n, lo, hi;
   printf("Please provide n: \n");
   scanf("%d", &n);

   printf("Please provide lo: \n");
   scanf("%d", &lo);

   printf("Please provide hi: \n");
   scanf("%d", &hi);

   FILE * fp;

   fp = fopen("log.txt", "w");


   fprintf(fp, "%d", n);
   int x = -1;
   for (int i = 0; i < n; i++) {
      fprintf(fp, "\n");
      if (lo == hi) {
        x = lo;
      } else {
        x = lo + rand() % (hi - lo + 1);
      }

      fprintf(fp, "%d", x);
   }
   return 0;
}
