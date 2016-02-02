/*
 * HW #6 Part 2
 * ICS 143A
 * Author: Kyle Bendickson.
 * ID: 18118767
 *
 * Program to simulate the bankers algorithm
 */

#include <stdio.h>
#include <stdlib.h>


static int debug = 0;

int main(int argc, char** argv)
{

  int system_is_safe = 0;
  int algorithm_did_terminate = 0;

  int num_processes = 0;
  int num_resources = 0;
  int num_processes_completed = 0;  /* Number of processes run to completion */
  int* process_order_vector;        /* Order the processes are performed */
  int* max_resource_vector;         /* Total number of each resources available to the system */
  int* unallocated_resource_vector; /* Number of each resource currently not allocated to a process */
  int** current_resource_table;
  int** max_resource_claim_table;

  /* Get number of prcesses P */
  printf("Enter the number of processes: ");
  scanf("%d", &num_processes);

  /* Get number of resources R */
  printf("Enter the number of resources: ");
  scanf("%d", &num_resources);

  if (debug)
  {
    printf("\nNumber of processes: %d\nNumber of resources: %d\n", num_processes, num_resources);
  }

  /* Allocate space for max resource vector */
  max_resource_vector = malloc(sizeof(int) * num_resources);

  /* Allocate space for unallocated resource vector */
  unallocated_resource_vector = malloc(sizeof(int) * num_resources);

  /* Allocate space for the process order vector */
  process_order_vector = malloc(sizeof(int) * num_processes);

  /* Allocate space for current resource allocation table */
  current_resource_table = malloc(sizeof(int*) * num_processes);
  for (int i = 0; i < num_processes; ++i)
    current_resource_table[i] = malloc(sizeof(int) * num_resources);

  /* Allocate space for max resource claim table. */
  max_resource_claim_table = malloc(sizeof(int*) * num_processes);
  for (int i = 0; i < num_processes; ++i)
    max_resource_claim_table[i] = malloc(sizeof(int) * num_resources);


  /* Get max resource vector 
   * - length R vector representing total number of 
   *    each resource available to the system.
   */
  printf("\nEnter the total number of each resource available to the system: ");
  for (int i = 0; i < num_resources; ++i)
  {
    scanf("%d", &max_resource_vector[i]);

    if (debug)
    {
      printf("Max for resource %d: %d\n", i, max_resource_vector[i]);
    }
  }


  /* Get current resource allocation table 
   * - P x R matrix representing how many of 
   *   each resource is currently allocated to each process.
   */
  printf("\n");  // Blank line is for formatting purposes.
  for (int i = 0; i < num_processes; ++i)
  {
    printf("Enter the total resources allocated for process%d: ", i);
    for (int j = 0; j < num_resources; ++j)
    {
      scanf("%d", &current_resource_table[i][j]);

      if (debug)
        printf("Current resources for process%d resource%d: %d\n", i, j, current_resource_table[i][j]);
      
    }
  }


  /* Get maximum resource claim table
   * - P x R matrix representing the max number of each resource
   *   that each process could ever require.
   */
  printf("\n"); // Blank line is for formatting purposes.
  for (int i = 0; i < num_processes; ++i)
  {
    printf("Enter the maximum resources needed for process%d: ", i);
    for (int j = 0; j < num_resources; ++j)
    {
      scanf("%d", &max_resource_claim_table[i][j]);

      if (debug)
        printf("Current resources for process%d resource%d: %d\n", i, j, max_resource_claim_table[i][j]);
      
    }
  }


  /* Calculate the current free resrouces vector
   * - length R vector representing the current number of 
   *   unclaimed resources, calculated from 
   */

  for (int i = 0; i < num_resources; ++i)
  {
    unallocated_resource_vector[i] = max_resource_vector[i];

    for (int p = 0; p < num_processes; ++p)
      unallocated_resource_vector[i] = unallocated_resource_vector[i] - current_resource_table[p][i];
  }


  if (debug)
  {
    printf("\nThe currently unallocated resource totals are: \n");
    for (int i = 0; i < num_resources; ++i)
      printf("%d | ", unallocated_resource_vector[i]);

    printf("\n\nThe currently allocated resources for each process are: \n");
    for (int p = 0; p < num_processes; ++p)
    {
      printf("%d: ", p);

      for (int r = 0; r < num_resources; ++r)
      {
        printf("%d | ", current_resource_table[p][r]);
      }

      printf("\n");
    }


    printf("\nThe max resources needed for each process are: \n");
    for (int p = 0; p < num_processes; ++p)
    {
      printf("%d: ", p);

      for (int r = 0; r < num_resources; ++r)
      {
        printf("%d | ", max_resource_claim_table[p][r]);
      }

      printf("\n");
    }

  } /* END if (debug) */


  /* After calculation, if any value in current free is -1 (from max - all allocated), panic! */
  for (int i = 0; i < num_resources; ++i)
  {
    if (unallocated_resource_vector[i] < 0)  // The inputs were ill formed.
    {
      printf("ERROR: The system as entered is ill-formed.\n");
      printf("The number of allocated resources for resource %d exceeds the system-wide maximum.\n", i);
      printf("The system is in an unsafe state.");
      exit(-1);
    }
  }



  /* Run algorithm, checking from p0 to pP.
   * At the first process that can be performed,
   * allocate the resources for that process and then
   * restart checking from p0.
   *
   * The algorithm terminates when all processes are
   * checked during the for loop without any of them running.
   */
  int num_loops = 0;
  while(!algorithm_did_terminate) 
  {
    
    int num_processes_checked = 0;      /* Number of processes checked for completion this iteration */

    for (int i = 0; i < num_processes; ++i)
    {
      num_processes_checked = num_processes_checked + 1;

      if (debug)
        printf("\nChecking process %d.", i);

      /* Fail early if the process has already been completed */
      if (current_resource_table[i][0] == -1)
      {
        continue;
      }

      int process_can_run = 0;
      for (int j = 0; j < num_resources; ++j)
      {

        if (debug)
          printf("\n\tChecking resource %d.", j);

        if (max_resource_claim_table[i][j] > (unallocated_resource_vector[j] + current_resource_table[i][j]))
        {

          if (debug)
            printf("\n\tFAILED ON RESOURCE %d", j);

          break;
        }
        else if (j == (num_resources - 1))
        {
          process_can_run = 1;
        }

      } /* END: for-each resource in process */

      if (process_can_run)
      {
        for (int r = 0; r < num_resources; ++r)
        {
          unallocated_resource_vector[r] += current_resource_table[i][r];
          current_resource_table[i][r] = -1;           // Indicates process did run.
        }

        process_order_vector[num_processes_completed] = i;
        ++num_processes_completed;

        break;

      } /* END if (process_can_run) */

    } /* END for-each processes loop. */


    /* Terminate the algorithm when all processes have been examined in one loop */
    if (num_processes_checked == num_processes)
    {
      algorithm_did_terminate = 1;
    }

  } /* END while (!algorithm_did_terminate). */


  /* Determine if all fo the processes completed */
  if (num_processes_completed == num_processes)
    system_is_safe = 1;

  /* Output if the system is in a safe state. */
  if (system_is_safe)
  {
    printf("\n\nThe processes completed in the following order: ");
    for (int i = 0; i < num_processes_completed; ++i)
      printf("%d ", process_order_vector[i]);
    printf("\n");
    printf("The system is in a safe state.");
  }
  else
    printf("\nThe system is in an unsafe state.");

  /* Deallocate vectors */
  free(max_resource_vector);
  free(process_order_vector);
  free(unallocated_resource_vector);

  /* Deallocate current resource table */
  for (int i = 0; i < num_processes; ++i)
    free(current_resource_table[i]);
  free(current_resource_table);

  /* Deallocate max resource claim table */
  for (int i = 0; i < num_processes; ++i)
    free(max_resource_claim_table[i]);
  free(max_resource_claim_table);

  return 0;
}
