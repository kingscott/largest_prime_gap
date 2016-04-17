#include <mpi.h>

#define WORKTAG           1
#define DEADTAG           2
#define ARRAY_SIZE        100

int compute(int input){
    // function to compute at input
    if (input % 42 != 0) {
        return (input % 42); 
    } else {
        return 0;
    }
}

int main( int argc, char * argv[]){
    
    int rank; 
    
    /* initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (rank == 0) {
        // exec master
        master();
        
        } else {
        // exec slave
        slave();
    }
    
    /* clean up MPI */
    MPI_Finalize();
}

master()
{
    int             n_cpu, rank, work;
    double          result;
    MPI_Status      status;

    int pixel_array[ARRAY_SIZE];     // 10 x 10 grid
    int pixel_index = ARRAY_SIZE;    // pixel count to 0 = finished
    
    /* get number of allocated CPU's */
    MPI_Comm_size(MPI_COMM_WORLD, &n_cpu); 
    printf("MPI launched with %d nodes\n", n_cpu);
    
    //  initialize all nodes with work
    for (rank = 1; rank < n_cpu; ++rank) {
        
        work = pixel_index;
        
        MPI_Send(&work,         /* pixel index */
        1,                      /* one data item */
        MPI_INT,                /* integer datatype */
        rank,                   /* destination in index rank of n_cpu */
        WORKTAG,                /* worktag */
        MPI_COMM_WORLD);        /* always use this */
        pixel_index--;          /* decrement pixel count */
    }
    
    /* Receive a result from any slave and dispatch a new work request */
    
    while (pixel_index >= 1) {
        
        printf("%d pixels left to compute\n", pixel_index);

        int buffer[2];
        int pixel_i, pixel_d;
        
        MPI_Recv(buffer,            /* result buffer */
        2,                          /* 0 = pixel index, 1 = pixel data */
        MPI_INT,                    /* data item is a double real */
        MPI_ANY_SOURCE,             /* receive from any sender */
        MPI_ANY_TAG,                /* receive any type of message */
        MPI_COMM_WORLD,             /* always use this */
        &status);                   /* info about received message */

        pixel_i = buffer[0];
        pixel_d = buffer[1];

        printf("Computed value of %d at pixel %d\n", pixel_d, pixel_i);
        pixel_array[pixel_i] = pixel_d;

        pixel_index--;              /* decrement index */
        work = pixel_index;         

        // send more work back to slave
        MPI_Send(&work, 1, MPI_INT, status.MPI_SOURCE, WORKTAG, MPI_COMM_WORLD);
        
        //work = pixel_index;
        //pixel_index--;
    }
    
    // Receive results for outstanding work requests.
    for(rank = 1; rank < n_cpu; ++rank) {

        int buffer[2];
        int pixel_i, pixel_d;

        MPI_Recv(buffer, 2, MPI_INT, MPI_ANY_SOURCE,
        MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pixel_i = buffer[0];
        pixel_d = buffer[1];

        printf("Computed value of %d at pixel %d\n", pixel_i, pixel_d);
        pixel_array[pixel_i] = pixel_d;

    }
    
    // Tell all the slaves to exit.
    for(rank = 1; rank < n_cpu; ++rank) {
        MPI_Send(0, 0, MPI_INT, rank, DEADTAG, MPI_COMM_WORLD);
    }

    int i;
    for(i = 0; i < ARRAY_SIZE; i++){
        printf("\t[%d : %d] ", i, pixel_array[i]);
        if(i > 0 && i % 5 == 0){
            printf("\n");
        }
    }
    printf("\n");
}

slave()

{
    int         result;
    int         work;
    MPI_Status  status;
    
    for (;;) {

        int buffer[2];
        int pixel_data;

        MPI_Recv(&work, 1, MPI_INT, 0, MPI_ANY_TAG,
        MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == DEADTAG) {
            return;
        }

        pixel_data = compute(work);

        buffer[0] = work;
        buffer[1] = pixel_data;
        
        MPI_Send(buffer, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
}
