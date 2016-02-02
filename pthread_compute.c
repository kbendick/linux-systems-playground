/**
 * CS143a - Fall 2015
 * HW #4
 * @author: Kyle Bendickson
 * pthread_compute.c
 *
 * This program reads in a file of numbers from stdin until the
 * end of the file and computes the min, the max, and the average.
 * The results are each printed out on a single line of output.
 * This program uses pthreads to split the computational work across
 * four threads (counting the original process thread containing main as one of the four).
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // Using exit()
#include <float.h>  // Using DBL_MAX, DBL_MIN

#define BUFFER_SIZE 1024
#define NUM_THREADS 4
// #define DEBUG       1   // Uncomment to add debugging statements.

static int NUM_SPAWNED_THREADS = NUM_THREADS - 1;

struct thread_data {
  int      thread_id;
  int      start_cell;
  int      end_cell;
  int      num_inputs_processed;
  double   min;
  double   max;
  double   sum;
  double*  buffer;              // pointer to an array of doubles.
};

void *process_data(void* thread_arg_as_void)
{
  struct thread_data *thread_arg = (struct thread_data*) thread_arg_as_void;

  for (int i = (int)thread_arg->start_cell; i < (int) thread_arg->end_cell; ++i)
  {
    double value = (double) thread_arg->buffer[i];
    thread_arg->sum += value;
    thread_arg->num_inputs_processed += 1;

    if(value < thread_arg->min)
    {
      thread_arg->min = value;
    }

    if(value > thread_arg->max)
    {
      thread_arg->max = value;
    }
  }

  pthread_exit((void*) 0);
}

int main(int argc, char** args)
{

  // Declare / Initialize variables  
  long int numInputs = 0;
  long int numInputsProcessed = 0;
  double input = 0.0;
  double sumOfInput = 0.0;
  double avg = 0.0;
  double minInput = DBL_MAX;
  double maxInput = DBL_MIN;

  double input_buffer[BUFFER_SIZE];

  pthread_t thread[NUM_THREADS - 1];                 // Using main process thread as one of computational threads.
  struct thread_data thread_data_array[NUM_THREADS]; // Includes data to be processed by "main" thread


  // Read all the numbers from the file into the buffer
  while(scanf("%lf", &input) != EOF)
  {
    input_buffer[numInputs] = input;
    ++numInputs;
  }

  // Determine the number of inputs sent to each thread.
  int numInputsPerThread = numInputs / NUM_THREADS;
  int extraInputsAfterSplit = numInputs % NUM_THREADS;
  
  // Initialize the thread_data_array with their start and end cell values
  for (int i = 0; i < NUM_THREADS; ++i)
  {
    thread_data_array[i].thread_id = i;
    thread_data_array[i].buffer = input_buffer;
    thread_data_array[i].min = DBL_MAX;
    thread_data_array[i].max = DBL_MIN;
    thread_data_array[i].sum = 0;
    thread_data_array[i].num_inputs_processed = 0;

    if (i==0)
    {
      thread_data_array[0].start_cell = 0;
      thread_data_array[0].end_cell = numInputsPerThread;
    }
    else
    {
      thread_data_array[i].start_cell = thread_data_array[i-1].end_cell;
      thread_data_array[i].end_cell = thread_data_array[i].start_cell + numInputsPerThread;
    }

    // Add remaining values to the last thread.
    if (i == (NUM_THREADS -1)) 
    {
      thread_data_array[i].end_cell += extraInputsAfterSplit;
    }


#ifdef DEBUG
    printf("thread %d has [start cell, end_cell] = [%d, %d]\n", i, thread_data_array[i].start_cell, thread_data_array[i].end_cell);
#endif

  }

  // Spawn threads.
  for (int i = 0; i < NUM_SPAWNED_THREADS; ++i)
  {
      int rc = pthread_create(&thread[i], NULL, process_data, (void*)&thread_data_array[i]); 
      if (rc) 
      {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
  }


  // Process data from main.
  for (int i = thread_data_array[NUM_THREADS-1].start_cell; i < thread_data_array[NUM_THREADS-1].end_cell; ++i)
  {
    double value = thread_data_array[NUM_THREADS-1].buffer[i];
    thread_data_array[NUM_THREADS-1].sum += value;
    thread_data_array[NUM_THREADS-1].num_inputs_processed += 1;

    if(value < thread_data_array[NUM_THREADS-1].min)
    {
      thread_data_array[NUM_THREADS-1].min = value;
    }

    if(value > thread_data_array[NUM_THREADS-1].max)
    {
      thread_data_array[NUM_THREADS-1].max = value;
    }
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
  
  return 0;
}
