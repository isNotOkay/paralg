
#include <stdio.h>
#include <math.h>
#include <omp.h>

/*
This file is used for both the sequential and parallel version.
The sequential version doesn't use openmp pragmas.
*/

typedef double atype_t;
static atype_t sum(atype_t x, atype_t y);
void print_array(int threadID, double x[], int size);
void balanced_tree(int n1, atype_t values[n1], atype_t (*f)(atype_t x, atype_t y));

int main()
{
	int n = 8;
	int n1 = 2*n-1; // leafs + inner elements + root
	atype_t values[n1];
	int i;

	// fill array with values 1...n
	int x = 1;
	for (i=n1-n; i<n1; i++) {
		values[i] = x;
		x++;
	}

	// calculate sum
	balanced_tree(n1,values,sum);
	print_array(0,values,n1);
	return 0;
}

/////////////////////////////////////////////////////////////
atype_t sum(atype_t x, atype_t y) {
	return x+y;
}

////////////////////////////////////////////////////////////////////////////////////

/*
Reduction with intermediate results
*/
void balanced_tree(int n1, atype_t values[n1], atype_t (*f)(atype_t x, atype_t y)) {
	int leafs = n1/2+1;
	int stride, i; 
	for (stride=(leafs/2); stride>0; stride/=2) {
		#pragma omp parallel for
		for (i=0; i<stride; i++) {
			values[stride+i-1] = f(values[2*stride+i-1], values[3*stride+i-1]);
		}
	}
}

void print_array(int threadID, double x[], int size) {
	printf("--- Array of thread with ID %d ---\n", threadID);
	int i;
	for (i=0;i<size;i++) {
		printf("%d: %.2f\n",i,x[i]);
	}
};