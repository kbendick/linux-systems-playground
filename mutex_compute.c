/**
 * CS143a - Fall 2015
 * HW #4
 * @author: Kyle Bendickson
 * mutex_compute.c
 *
 * This program reads in a file of numbers from stdin until the
 * end of the file and computes the min, the max, and the average.
 * The results are each printed out on a single line of output.
 * This program uses pthreads to split the computational work across
 * four threads and uses a mutex to control which thread is able to 
 * read in data from stdin at any given time.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // Using exit()
#include <float.h>  // Using DBL_MAX, DBL_MIN

#define BUFFER_SIZE 1024
#define NUM_THREADS 4

static int NUM_SPAWNED_THREADS = NUM_THREADS - 1;
static long int numInputs = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct thread_data {
  int      thread_id;
  int      num_inputs_processed;
  double   min;
  double   max;
  double   sum;
  double*  value;
};


void *get_and_process_data(void* thread_arg_as_void)
{
  struct thread_data *thread_arg = (struct thread_data*) thread_arg_as_void;
  while (1) 
  {
    pthread_mutex_lock(&mutex);
    if (scanf("%lf", thread_arg->value) != EOF)
    { 

      double value = *thread_arg->value;
      pthread_mutex_unlock(&mutex);
      thread_arg->sum += value;
      thread_arg->num_inputs_processed += 1;
      numInputs++;

      if(value < thread_arg->min)
      {
        thread_arg->min = value;
      }

      if(value > thread_arg->max)
      {
        thread_arg->max = value;
      }
    }
    else 
    {
      pthread_mutex_unlock(&mutex);
      break;
    }
  }

  if( thread_arg->thread_id != NUM_SPAWNED_THREADS)
    pthread_exit((void*) 0);
  else
    return 0;
}

int main(int argc, char** args)
{

  // Declare / Initialize variables  
  long int numInputsProcessed = 0;
  double input = 0.0;
  double sumOfInput = 0.0;
  double avg = 0.0;
  double minInput = DBL_MAX;
  double maxInput = DBL_MIN;

  double input_buffer[BUFFER_SIZE];

  pthread_t thread[NUM_SPAWNED_THREADS];              // Using main process thread as one of computational threads.
  struct thread_data thread_data_array[NUM_THREADS]; // Includes data to be processed by "main" thread

  // Initialize the thread_data_array with their values.
  double* value_ptr = malloc(sizeof(double));
  for (int i = 0; i < NUM_THREADS; ++i)
  {
    thread_data_array[i].thread_id = i;
    thread_data_array[i].min = DBL_MAX;
    thread_data_array[i].max = DBL_MIN;
    thread_data_array[i].sum = 0;
    thread_data_array[i].num_inputs_processed = 0;
    thread_data_array[i].value = value_ptr;
  }

  // Spawn threads and process data.
  for (int i = 0; i < NUM_THREADS; ++i)
  {
    if (i != NUM_SPAWNED_THREADS)
    {
      int rc = pthread_create(&thread[i], NULL, get_and_process_data, (void*)&thread_data_array[i]); 
      if (rc) 
      {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
    }
    else
      get_and_process_data((void*)&thread_data_array[i]);
  }

  // Join all threads.
  for (int t = 0; t < NUM_SPAWNED_THREADS; ++t)
  {
    pthread_join(thread[t], NULL);
  }


  // Process the data.
  for (int i = 0; i < NUM_THREADS; ++i)
  {
    sumOfInput += thread_data_array[i].sum;
    numInputsProcessed += thread_data_array[i].num_inputs_processed;

    if (thread_data_array[i].min < minInput)
      minInput = thread_data_array[i].min;

    if (thread_data_array[i].max > maxInput)
      maxInput = thread_data_array[i].max;
  }

  // Some error-checking if the file was empty
  if (numInputs == 0)
  {
    printf("No numbers found in file!\n");
    exit(1);
  }

  // Verify that all of the inputs were processed.
  if (numInputsProcessed != numInputs || numInputsProcessed == 0)
  {
    printf("Something went wrong... Not all of the inputs were processed!\n");
    printf("Expected %li inputs but only processed %li\n", numInputs, numInputsProcessed);
    exit(2);
  }

  // Compute the average
  avg = sumOfInput / numInputsProcessed;

  // Print the results
  printf("max: %lf\nmin: %lf\naverage: %lf\n", maxInput, minInput, avg);

  free(value_ptr);  
  return 0;
}
