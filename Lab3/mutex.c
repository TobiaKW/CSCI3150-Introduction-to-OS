#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define NITERS 10
#define NUM_THREADS 8
unsigned int cnt = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
/* thread function */
void* count(void* arg) {
  int i;
  for(i = 0; i < NITERS; i++) {
    pthread_mutex_lock(&mtx);
    cnt++;
    pthread_mutex_unlock(&mtx);
  }
  pthread_exit(NULL);
}
/* main */
int main()
{
  pthread_t thr[NUM_THREADS];
  int i, rc;
  /* create threads */
  for (i = 0; i < NUM_THREADS; ++i) {
    if (rc == pthread_create(&thr[i], NULL, count, NULL)) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  /* block until all threads complete */
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join(thr[i], NULL);
  }
  printf ("Total count: %d\n", cnt);
  return EXIT_SUCCESS;
}