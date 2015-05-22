#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "libFHBRS/libFHBRS.h"

int cmpfunc(const void *a, const void *b);
void merge(int x[], int y[], int z[], int length);
int is_sorted(int x[], int length);

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Wrong number of arguments. Expecting: <n>\n");
    return -1;
  }
  int n = atoi(argv[1]);
  int i, j, blocksize, rank, num_processors;
  int *values;
  int *block; 
  int *block_neighbour;
  int *block_merged;
  double t0, t1;
  ////////////////////////////////////////////////////////
  t0 = gettime();
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("%d: \n",rank);
  blocksize = n/num_processors;
  block           = malloc(blocksize*sizeof(int));
  block_neighbour = malloc(blocksize*sizeof(int));
  block_merged    = malloc(2*blocksize*sizeof(int));

  
  if (rank == 0) {
    // allocate memory and fill array with values in reverse order
    values = malloc(n*sizeof(int));
    for (i=0;i<n;i++) {
      values[i] = n-i;
    }
    memcpy(block,values,blocksize*sizeof(int));

    // split values among all processors
    int *start = &values[blocksize];
    for (i=1;i<num_processors;i++) {
      MPI_Send(start, blocksize, MPI_INT, i, 0, MPI_COMM_WORLD);
      start += blocksize;
    }
  } else {
    MPI_Recv(block, blocksize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  // every processor sorts its assigned block 
  qsort(block, blocksize, sizeof(int), cmpfunc);
  assert(is_sorted(block, blocksize));

  ///// OK //////////////////////////

  // // merge and split on two neighbouring processors
  for (j=0;j<num_processors-1;j++) {
    if ((rank%2) == (j%2)) {
      MPI_Recv(block_neighbour, blocksize, MPI_INT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      merge(block, block_neighbour, block_merged, blocksize);
      memcpy(block, block_merged, blocksize*sizeof(int));
      MPI_Send(block_merged+blocksize, blocksize, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
    } else {
      MPI_Send(block, blocksize, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
      MPI_Recv(block, blocksize, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }


  // TODO: aggregate all blocks in values //

  if (rank == 0) {
    for (i=1;i<num_processors;i++) {
      MPI_Recv(block, blocksize, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      memcpy(values, block+i*blocksize, blocksize*sizeof(int));
      assert(is_sorted(values, n));
    } 
  } else {
   MPI_Send(block, blocksize, MPI_INT, 0, 0, MPI_COMM_WORLD);
 }
 




 assert(is_sorted(block, blocksize));
 if (rank == 0) {
  t1 = gettime();
  printf("\nCalculation took %f seconds.\n",t1-t0);
}







MPI_Finalize();
return 0;
}

///////////////////////////////////////////////////////////

/* 
returns negative if b > a 
and positive if a > b 
*/  
int cmpfunc(const void *a, const void *b) { 
    const int *x = (const int *)a; // casting pointer types 
    const int *y = (const int *)b;
    return *x  - *y;  
  } 

  void merge(int x[], int y[], int z[], int length) {
    int pos_x = 0;
    int pos_y = 0;
    int pos_z;
    for (pos_z=0;pos_z<(length*2);pos_z++) {
      if (pos_x == length) {
        z[pos_z] = y[pos_y++];
      } else if (pos_y == length) {
        z[pos_z] = x[pos_x++];
      } else if (x[pos_x] < y[pos_y]) {
        z[pos_z] = x[pos_x++];
      } else {
        z[pos_z] = y[pos_y++];
      }
    }
  }

  int is_sorted(int x[], int length) {
    int i;
    for (i=0;i<length-1;i++) {
      if (x[i+1] < x[i]) {
        return 0;
      }
    }
    return 1;
  }
