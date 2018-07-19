#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <cblas.h>
#include "matrix.h"
//#include <chrono>
//#include <iostream>

int main(int argc, char* argv[])
{
  int i;
  int m = 785;
  int n = 512;
  int k = 784;
  int sizeofa = m * k;
  int sizeofb = k * n;
  int sizeofc = m * n;
  float alpha = 1.2;
  float beta = 0.001;

  struct timeval start,finish;
  float duration;

  float* a = (float*)malloc(sizeof(float) * sizeofa);
  float* b = (float*)malloc(sizeof(float) * sizeofb);
  float* c = (float*)malloc(sizeof(float) * sizeofc);
  float* a1 = (float*)malloc(sizeof(float) * sizeofa);
  float* b1 = (float*)malloc(sizeof(float) * sizeofb);
  float* c1 = (float*)malloc(sizeof(float) * sizeofc);

  srand((unsigned)time(NULL));

  for (i=0; i<sizeofa; i++)
    a[i] = i+1;//(rand()%100)/10.0;

  for (i=0; i<sizeofb; i++)
    b[i] = i+1;//(rand()%100)/10.0;

  for (i=0; i<sizeofc; i++)
    c[i] = i+1;//(rand()%100)/10.0;

  struct Matrix A = {a, m, k};
  struct Matrix B = {b, k, n};
  struct Matrix C = {c, m, n};

  gettimeofday(&start, NULL);
  //auto s = std::chrono::system_clock::now();
  for(i=0;i<1;i++){
    matmul(&A, &B, &C);
  }
  gettimeofday(&finish, NULL);
  //auto stop = std::chrono::system_clock::now();
  
  //std::cout << "Elasped time is "
  //            << std::chrono::duration<float>{stop - s}.count()
  //            << " seconds." << std::endl;
  
  duration = ((float)(finish.tv_sec-start.tv_sec)*1000000 + (float)(finish.tv_usec-start.tv_usec)) / 1000000;
  printf("\n%lf\n",duration);

  free(a);
  free(b);
  free(c);
  return 0;
}
