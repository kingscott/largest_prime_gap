#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gmp.h>
#include "mpi.h"

#define MIN(a,b) (a < b ? a : b)

int main(int argc, char **argv) {
  int my_rank;
  int num_procs = 4;
  int source;
  int dest;
  int bound = 1000;
  mpz_t biggest_diff;
  mpz_init(biggest_diff);

  MPI_Init(&argc, &argv); // start mpi
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // process rank
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // find out the number of process

  int chunk = floor(bound/num_procs);
  int r = fmod(bound, num_procs);

  double index = my_rank*chunk + MIN(my_rank, r);
  double i = index;
  mpz_t m_index;
  mpz_init_set_d(m_index, index);

  mpz_t prev_prime;
  mpz_init(prev_prime);
  mpz_t curr_prime;
  mpz_init(curr_prime);

  mpz_nextprime(prev_prime, m_index);
  printf("My rank: %d \t", my_rank);
  gmp_printf("Starting prime: %Zd \n\n", prev_prime);

  while (i < index + chunk) {
    mpz_nextprime(curr_prime, prev_prime);
    printf("My rank: %d \t", my_rank);
    i = mpz_get_d(curr_prime) + 1;
    mpz_sub(biggest_diff, curr_prime, prev_prime);
    gmp_printf("%Zd \t %Zd\n\n", curr_prime, biggest_diff);
    mpz_set(prev_prime, curr_prime);
  }

  MPI_Finalize();
}
