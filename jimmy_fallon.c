#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>

static sem_t lock;           // for locking id
static int next_id = 1;      //id
static sem_t connected_lock; // for locking connected lines
static sem_t operators;      // for operators
static int NUM_OPERATORS = 3;
static int NUM_LINES = 5;
static int connected = 0; // Callers that are connected

void *phonecall(void *vargp)
{
  int id;
  //to check if the caller can move on to connect
  _Bool enter = 0;

  //increment id and next_id
  sem_wait(&lock);
  id = next_id++;
  sem_post(&lock);

  //inform which call is attempting to connect
  printf("Thread %i is attempting to connect\n", id);

  //while loop to allow caller to connect with the operator
  while (enter == 0)
  {
    sem_wait(&connected_lock);
    //if there is available line to connect, the program will change enter variable to get out of the while loop
    //and increment connected
    if (connected < NUM_LINES)
    {
      enter = 1;
      connected++;
    }
    sem_post(&connected_lock);

    //if it does not go into the previous condition, it indicates that there is no free line, will continue in this loop
    if (enter == 0)
    {
      printf("Thread %i is calling line, busy signal\n", id);
      sleep(1);
    }
    //if there is available line to connect, the program will inform this
    else
    {
      printf("Thread %i has available line, call ringing\n", id);
    }
  }

  //section for talking to operator
  sem_wait(&operators);
  printf("Thread %i is speaking to operator\n", id);

  //wait for some seconds
  sleep(3);
  printf("Thread %i has bought a ticket!\n", id);
  sem_post(&operators);

  //decrease connected lines
  sem_wait(&connected_lock);
  connected--;
  sem_post(&connected_lock);

  //finish the call
  printf("Thread %i has hung up!\n", id);
}

int main(int argc, char *argv[])
{
  void *phonecall(void *vargp);
  //initialize semaphores for 3 variables
  sem_init(&connected_lock, 0, 1);        //binary semaphores
  sem_init(&lock, 0, 1);                  //binary semaphores
  sem_init(&operators, 0, NUM_OPERATORS); //counting semaphores

  //read from input:
  int CALLS = atoi(argv[1]);
  //as we do not accept cases that are negative or more than 254.
  if (CALLS <= 0 || CALLS > 240)
  {
    exit(0);
  }
  //array to store callers
  pthread_t tid[CALLS];
  //loop to create threads
  for (int i = 0; i < CALLS; i++)
  {
    pthread_create(&tid[i], NULL, phonecall, NULL);
  }
  //loop to join the threads
  for (int j = 0; j < CALLS; j++)
  {
    pthread_join(tid[j], NULL);
  }

  //destroy the semaphores
  sem_destroy(&connected_lock);
  sem_destroy(&operators);
  sem_destroy(&lock);

  return 0;
}
