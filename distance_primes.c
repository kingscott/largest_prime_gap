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
  long long bound = 1000000000000;
  double largest_diff = 0;
  double temp_diff = 0;
  MPI_Status  status;

  //initialize MPI
  MPI_Init(&argc, &argv); // start mpi
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // process rank
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // find out the number of process
  MPI_Barrier(MPI_COMM_WORLD);
  double elapsed_time = -MPI_Wtime();

  //calculate chunk size per processor and the remainder
  long long int chunk = floor(bound/num_procs);
  long long int r = fmod(bound, num_procs);

  //determine the starting index for a given processor and the next
  long long index = my_rank * chunk + MIN(my_rank, r);
  long long next_start_index = (my_rank + 1) * chunk + MIN((my_rank + 1), r);
  long long i = index;
  

  //initialize mpz types
  mpz_t key_prime_1;
  mpz_init_set_d(key_prime_1, 2);
  mpz_t key_prime_2;
  mpz_init_set_d(key_prime_2, 2);
  
  mpz_t m_index;
  mpz_init_set_d(m_index, index);
  mpz_t diff;
  mpz_init(diff);
  mpz_t prev_prime;
  mpz_init(prev_prime);
  mpz_t curr_prime;
  mpz_init(curr_prime);

  printf("My rank: %d \t %.0lli \t STARTING INDEX\n", my_rank, index);
  mpz_nextprime(prev_prime, m_index);
  mpz_set(curr_prime, prev_prime);
  i = mpz_get_d(prev_prime) + 1;

  //loop getting primes and calculating differences until the prime is
  //outside of this processor's chunk size
  while (i < next_start_index) {
    //subtract current prime from previous
    mpz_sub(diff, curr_prime, prev_prime);
    temp_diff = mpz_get_d(diff);

    //if this is the largest difference, store it
    if (temp_diff > largest_diff) {
      mpz_set(key_prime_1, prev_prime);
      mpz_set(key_prime_2, curr_prime);
      largest_diff = temp_diff;
    }

    //if this is the largest difference, store it
    if (temp_diff > largest_diff) {
      largest_diff = temp_diff;
    }

    //set the previous prime to the current and get the next prime
    mpz_set(prev_prime, curr_prime);
    mpz_nextprime(curr_prime, prev_prime);
    i = mpz_get_d(curr_prime) + 1;
  }

  //subtract the previous prime from the current to get difference
  mpz_sub(diff, curr_prime, prev_prime);
  temp_diff = mpz_get_d(diff);

  //if this is the largest difference, store it
  if (temp_diff > largest_diff) {
    mpz_set(key_prime_1, prev_prime);
    mpz_set(key_prime_2, curr_prime);
    largest_diff = temp_diff;
  }
  
  //print data
  printf("My rank: %d \t ", my_rank);
  gmp_printf("%Zd \t LAST PRIME\n", prev_prime);
  printf("My rank: %d \t %.0f \t LARGEST DIFF\n", my_rank, largest_diff);

  //if this is proc 0, listen for differences from other procs
  if (my_rank == 0) {
    double proc_diff;
    long long prime_1 = mpz_get_d(key_prime_1);
    long long prime_2 = mpz_get_d(key_prime_2);
    long long tmp_prime_1 = 0;
    long long tmp_prime_2 = 0;

    //get difference from each proc and determine largest
    for (source = 1; source < num_procs; source++) {
      MPI_Recv(&proc_diff, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, &status);
      MPI_Recv(&tmp_prime_1, 1, MPI_LONG_LONG, source, 1, MPI_COMM_WORLD, &status);
      MPI_Recv(&tmp_prime_2, 1, MPI_LONG_LONG, source, 2, MPI_COMM_WORLD, &status);
      if (proc_diff > largest_diff) {
        prime_1 = tmp_prime_1;
        prime_2 = tmp_prime_2;
        largest_diff = proc_diff;
      }
    }

    //print data
    printf("FINAL LARGEST DIFF: %.0f \t between: %.0f and %.0f\n", largest_diff, prime_1, prime_2);
  }
  else {
    long long prime_1 = mpz_get_d(key_prime_1);
    long long prime_2 = mpz_get_d(key_prime_2);
    
    MPI_Send(&largest_diff, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    MPI_Send(&prime_1, 1, MPI_LONG_LONG, 0, 1, MPI_COMM_WORLD);
    MPI_Send(&prime_2, 1, MPI_LONG_LONG, 0, 2, MPI_COMM_WORLD);
  }
  
  elapsed_time = MPI_Wtime();
  printf("It took %lf\n", elapsed_time);

  MPI_Finalize();
}
