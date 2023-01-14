// Program for sorting a list of inputs
#include <stdio.h>

void merge(int* newArray, int* arr1, int* arr2, int arr1_size, int arr2_size) {


   int indx1 = 0;
   int indx2 = 0;

   while (indx1 < arr1_size && indx2 < arr2_size) {

      if (arr1[indx1] > arr2[indx2]) {
         newArray[indx1 + indx2] = arr2[indx2];
         indx2++;
      } else {
         newArray[indx1 + indx2] = arr1[indx2];
         indx1++;
      }
   }

   for (indx1; indx1 < arr1_size; indx1++) {
      newArray[indx1 + indx2] = arr1[indx1];
   }

   for (indx2; indx2 < arr2_size; indx2++) {
      newArray[indx1 + indx2] = arr2[indx2];
   }

}

void sort(int* arr, int arr_size) {

   int lhs_arr_size = arr_size - arr_size/2;
   int rhs_arr_size = arr_size - lhs_arr_size;

   int lhs_arr [lhs_arr_size];
   int rhs_arr [rhs_arr_size];


   for (int i = 0; i < lhs_arr_size; i++) {
      lhs_arr[i] = arr[i];
   }

   for (int i = 0; i < rhs_arr_size; i++) {
      rhs_arr[i] = arr[lhs_arr_size + i];
   }


   if (lhs_arr_size <= 1 && rhs_arr_size <= 1) {
      merge(arr, lhs_arr, rhs_arr, lhs_arr_size, rhs_arr_size);
   } else {
      sort(lhs_arr, lhs_arr_size);
      sort(rhs_arr, rhs_arr_size);
      merge(arr, lhs_arr, rhs_arr, lhs_arr_size, rhs_arr_size);
   }

}

int main(void) {
   FILE * fp;

   fp = fopen("log.txt", "r");

   int n;

   fscanf(fp, "%d", &n);

   int numbersToSort [n];

   for (int i = 0; i < n; i++) {
      int num;
      fscanf(fp, "%d", &num);
      numbersToSort[i] = num;
   }

   sort(numbersToSort, n);

   FILE * fp1;

   fp1 = fopen("sorted.txt", "w");

   fprintf(fp1, "%d", n);
   for (int i = 0; i < n; i++) {
      fprintf(fp1, "\n%d", numbersToSort[i]);
   }

   fclose(fp);
   fclose(fp1);

   return 0;
}
