// Program for generting random integers between lo and hi

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main (void) {
   double n_init, lo_init, hi_init;
   printf("Please provide n: \n");
   scanf("%lf", &n_init);

   printf("Please provide lo: \n");
   scanf("%lf", &lo_init);

   printf("Please provide hi: \n");
   scanf("%lf", &hi_init);

   if (n_init > INT_MAX || hi_init > INT_MAX || lo_init < INT_MIN || lo_init > hi_init || n_init < 0) {
      return 1;
   }

   int n = (int) n_init;
   int lo = (int) lo_init;
   int hi = (int) hi_init;

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
   fclose(fp);
   return 0;
}
