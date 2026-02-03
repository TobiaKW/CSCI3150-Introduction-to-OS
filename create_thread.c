#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define NUM_THREADS 8
/* thread function */
void* thr_func(void* arg) {
  int data = *(int*)arg;
  printf("hello from thr_func, thread id: %d\n", data);
  pthread_exit(NULL);
}
/* data passed to thread */
int thr_idx[NUM_THREADS];
/* main */
int main(int argc, char* argv[])
{
  pthread_t thr[NUM_THREADS];
  int i, rc;
  /* create threads */
  for (i = 0; i < NUM_THREADS; ++i) {
    thr_idx[i] = i;
    if (rc == pthread_create(&thr[i], NULL, thr_func, &thr_idx[i])) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  /* block until all threads complete */
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join(thr[i], NULL);
  }
  printf("All threads exited.\n");
  return EXIT_SUCCESS;
}