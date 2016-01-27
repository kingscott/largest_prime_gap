#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gmp.h>
#include "mpi.h"

#define MIN(a,b) (a < b ? a : b)

int main(int argc, char **argv) {
  int my_rank;
  int num_procs;
  int source;
  int dest;
  int bound = 1000;
  double largest_diff = 0;
  double temp_diff = 0;
  mpz_t diff;
  mpz_init(diff);

  MPI_Init(&argc, &argv); // start mpi
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // process rank
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // find out the number of process

  int chunk = floor(bound/num_procs);
  int r = fmod(bound, num_procs);

  double index = my_rank * chunk + MIN(my_rank, r);
  double next_start_index = (my_rank + 1) * chunk + MIN((my_rank + 1), r);
  double i = index;
  mpz_t m_index;
  mpz_init_set_d(m_index, index);

  mpz_t prev_prime;
  mpz_init(prev_prime);
  mpz_t curr_prime;
  mpz_init(curr_prime);

  //initial
  printf("My rank: %d \tStarting index: %f\n\n", my_rank, index);
  mpz_nextprime(prev_prime, m_index);
  mpz_set(curr_prime, prev_prime);
  i = mpz_get_d(prev_prime) + 1;

  while (i < next_start_index) {           
    mpz_sub(diff, curr_prime, prev_prime);
    temp_diff = mpz_get_d(diff);
    if (temp_diff > largest_diff) {
      largest_diff = temp_diff;
    }
    
    //printf("My rank: %d \t", my_rank); 
    //gmp_printf("%Zd \t %Zd\n", curr_prime, diff);
    
    mpz_set(prev_prime, curr_prime);
    mpz_nextprime(curr_prime, prev_prime);   
    i = mpz_get_d(curr_prime) + 1;
  }
	
  mpz_sub(diff, curr_prime, prev_prime);
  printf("My rank: %d \t", my_rank);
  gmp_printf("%Zd \t %Zd \t LAST\n", curr_prime, diff);
  
  printf("Largest Difference: %f\n", largest_diff);

  MPI_Finalize();
}
