#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>


int main() {

	//initialisation
	MPI_Init(NULL, NULL);


    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size != 9) {
    	printf("There must be 9 processes. Exitting...\n");
    	MPI_Finalize();
    	exit(1);
    }


    
    int dims[] = {3, 3};
    int periods[] = {1, 1}; //all dimentions have wraparound connections
    MPI_Comm comm_cart;

    MPI_Cart_create(MPI_COMM_WORLD, /*ndims*/ 2, dims, periods, /*reorder*/ 1, &comm_cart);
    

    int myrank;
    MPI_Comm_rank(comm_cart, &myrank);

    int mycoords[2];
    MPI_Cart_coords(comm_cart, myrank, 2, mycoords);


    char mychar = 'A' + myrank;
    printf("(%d, %d) - %c\n", mycoords[0], mycoords[1], mychar);



    int rank_source, rank_dest;
    MPI_Cart_shift(comm_cart, /*direction*/ 1, /*step*/ 1, &rank_source, &rank_dest);



    MPI_Send((void *) &mychar, 1, MPI_CHAR, rank_dest, 0, comm_cart);

    char recvchar;
	MPI_Recv((void *) &recvchar, 1, MPI_CHAR, rank_source, 0, comm_cart, MPI_STATUS_IGNORE);

	printf("(%d, %d) - Received: %c\n", mycoords[0], mycoords[1], recvchar);



    //finalization
    MPI_Finalize();

}