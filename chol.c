#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "./papi/include/papi.h"
#include <stdbool.h> 

/* for Fortran - j*n + i */
//#define IDX(i, j, n)	(((i) * (n)) + (j))
#define IDX(i, j, n) (((j)+ (i)*(n)))
bool papi_supported = true; 
int events[5] = {PAPI_FP_OPS, PAPI_LD_INS, PAPI_SR_INS, PAPI_L1_DCM, PAPI_L2_TCM};
long long values[5] = {0,};
int eventSet = PAPI_NULL;
int papi_err;
void initialize() {
	if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
		fprintf(stderr, "PAPI is unsupported.\n");
		papi_supported = false;
	}

	if (PAPI_num_counters() < 5) {
		fprintf(stderr, "PAPI is unsupported.\n");
		papi_supported = false;
	}

	if ((papi_err = PAPI_create_eventset(&eventSet)) != PAPI_OK) {
		fprintf(stderr, "Could not create event set: %s\n", PAPI_strerror(papi_err));
	}
	int pp =0; 
	while (pp<5) {
		if ((papi_err = PAPI_add_event(eventSet, events[pp])) != PAPI_OK ) {
			fprintf(stderr, "Could not add event: %s\n", PAPI_strerror(papi_err));
		}
		pp++;
	}
}

/* start counters */
void start_c() {
	if (papi_supported) {
		if ((papi_err = PAPI_start(eventSet)) != PAPI_OK) {
			fprintf(stderr, "Could not start counters: %s\n", PAPI_strerror(papi_err));
		}
	}
}

/* stop conuters */

void stop_print() {
	if (papi_supported) {
		if ((papi_err = PAPI_stop(eventSet, values)) != PAPI_OK) {
			fprintf(stderr, "Could not get values: %s\n", PAPI_strerror(papi_err));
		}
		// PAPI_FP_OPS
		// PAPI_LD_INS
		// PAPI_SR_INS
		// PAPI_L1_DCM
		// PAPI_L2_TCM
		printf("Performance counters for factorization stage: \n");
		printf("\tFP OPS: %ld\n", values[0]);
		printf("\tLD INS: %ld\n", values[1]);
		printf("\tSR INS: %ld\n", values[2]);
		printf("\tL1 DCM: %ld\n", values[3]);
		printf("\tL2 TCM: %ld\n", values[4]);
	}
}


int
chol(double *A, unsigned int n)
{
	unsigned int i;
	unsigned int j;
	unsigned int k;
	double temp; 
	double sqr; 
	double value; 
	initialize();
	start_c();
	for (j = 0; j < n; j++) {
		for (i = j; i < n; i++) {
			value = 0; 
			for (k = 0; k < j;k++) {
				temp = 0.0; 

				if (i==j)
				{
					temp = A[IDX(i, k, n)];
					value += temp*temp;
					//temp = A[IDX(i, k+1, n)];
					//value += temp * temp; 
				}
				else {
					value += A[IDX(i, k, n)] * A[IDX(j, k, n)];
					//value += A[IDX(i, k+1, n)] * A[IDX(j, k+1, n)];
				}	 
			}
			//while (k<j) {
			//	value += A[IDX(i, k, n)] * A[IDX(j, k, n)];
			//	k++;
			//}
			A[IDX(i, j, n)] -= value; 
		}

		if (A[IDX(j, j, n)] < 0.0) {
			return (1);
		}
		sqr =0;
		sqr = sqrt(A[IDX(j, j, n)]);
		A[IDX(j, j, n)] = sqr; 
		value = sqr;
		for (i = j + 1; i < n; i++)
			A[IDX(i, j, n)] /= value;
	}
	stop_print();
	return (0);
}
double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}
int
main() //1/3 * /czas /milion
{
	double *A;
	int i, j, n, ret;
	srand(time(NULL));
	//int r = rand();
	n = 1000;
	A = calloc(n*n, sizeof(double));
	assert(A != NULL);

//	A[IDX(0, 0, n)] = 4.0;   A[IDX(0, 1, n)] = 12.0;  A[IDX(0, 2, n)] = -16.0;
//	A[IDX(1, 0, n)] = 12.0;  A[IDX(1, 1, n)] = 37.0;  A[IDX(1, 2, n)] = -43.0;
//	A[IDX(2, 0, n)] = -16.0; A[IDX(2, 1, n)] = -43.0; A[IDX(2, 2, n)] = 98.0;
	int kl = 0;
	int z=0;
	while (z<n)
	{
		//kl=0;
		//while(kl<n)
		//{
			A[IDX(z,z,n)]=fRand(0.00,500.00);
		//	kl++;		
		//}
		z++;
	}
	if (chol(A, n)) {
		fprintf(stderr, "Error: matrix is either not symmetric or not positive definite.\n");
	} else {
		fprintf(stdout, "Tri(L) = \n");
/*		for (i = 0; i < n; i++) {
			for (j = 0; j <= i; j++)
				printf("%2.8lf\t", A[IDX(i, j, n)]);
			printf("\n");
*/		//}
	}
//Krok testow co 100 
	free(A);
	return 0;
}

