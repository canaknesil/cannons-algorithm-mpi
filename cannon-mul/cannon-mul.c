#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "util/matrix.h"



float *mblock1;
float *mblock2;


void do_multiplication();


int main(int argc, char *argv[]) {


	//initialisation
	MPI_Init(NULL, NULL);

	//create cartesian topology
	int dims[] = {3, 3};
    int periods[] = {1, 1}; //all dimentions have wraparound connections
    MPI_Comm comm_cart;

    MPI_Cart_create(MPI_COMM_WORLD, /*ndims*/ 2, dims, periods, /*reorder*/ 1, &comm_cart);

    //getting information
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    int master_rank = 0;



    //getting arguments
    int org_row1 = 3; //default values of arguments
    int org_col1 = 3;
    int org_row2 = org_col1;
    int org_col2 = 3;

    int block_size = 1;

    int print_matrices = 0;
    int see_progress = 0;


	int c;
	while ((c = getopt(argc, argv, "s:prb:")) != -1) {

		switch (c) {

			case 's': 
				{
					int s = atoi(optarg);
					if (s > 0) {
						org_row1 = org_col1 = org_row2 = org_col2 = s;
					}
				}
				
				break;

			case 'p':
				print_matrices = 1;
				break;

			case 'r': 
				see_progress = 1;
				break;

			case 'b':
				{
					int bs = atoi(optarg);
					if (bs > 0) {
						block_size = bs;
					}
					break;
				}

			default:
				break;
				
		}

	}


	int expected_np = org_row1 * org_row1;
	if (world_size != expected_np) {
		if (myrank == master_rank) printf("Incorrect number of process. There must be %d process.\nQuitting...\n", expected_np);
		MPI_Finalize();
		exit(1);
	}



	//calculate extended dimentions
	int row1 = org_row1 + (org_row1 % block_size == 0 ? 0 : block_size - org_row1 % block_size);
	int col1 = org_col1 + (org_col1 % block_size == 0 ? 0 : block_size - org_col1 % block_size);
	int row2 = col1;
	int col2 = org_col2 + (org_col2 % block_size == 0 ? 0 : block_size - org_col2 % block_size);

	int bl_size_sq = block_size * block_size;



	//master process
	if (myrank == master_rank) {

		//srand(time(NULL));

		Matrix *mat1 = matrix_create_random(row1, col1);
		Matrix *mat2 = matrix_create_random(row2, col2);
		Matrix *product = matrix_create(row1, col2);
		Matrix *org_product = matrix_create(org_row1, org_col2);

		//fill extra space with zeros
		int i, j, k, l;
		for (i=0; i<row1; i++) {
			for (j = org_col1; j < col1; j++) {
				mat1->values[i][j] = 0;
			}
		}
		for (j=0; j<col1; j++) {
			for (i = org_row1; i < row1; i++) {
				mat1->values[i][j] = 0;
			}
		}

		for (i=0; i<row2; i++) {
			for (j = org_col2; j < col2; j++) {
				mat2->values[i][j] = 0;
			}
		}
		for (j=0; j<col2; j++) {
			for (i = org_row2; i < row2; i++) {
				mat2->values[i][j] = 0;
			}
		}

		//print matrices
		if (print_matrices) {

			printf("MAT 1:\n");
			matrix_print(mat1);
			printf("\n");

			printf("MAT 2:\n");
			matrix_print(mat2);
			printf("\n");

		}

		//send approproate blocks to processes
		float *block1 = malloc(bl_size_sq * sizeof(float));
		float *block2 = malloc(bl_size_sq * sizeof(float));

		mblock1 = malloc(bl_size_sq * sizeof(float));
		mblock2 = malloc(bl_size_sq * sizeof(float));

		for (i=0; i<row1/block_size; i++) {
			for (j=0; j<col2/block_size; j++) {
				//every block

				int start_row = i * block_size;
				int start_col = j * block_size;

				for (k=0; k<block_size; k++) {
					for (l=0; l<block_size; l++) {

						float *block1_ptr;
						float *block2_ptr;
						if (myrank == master_rank) {
							block1_ptr = mblock1;
							block2_ptr = mblock2;
						} else {
							block1_ptr = block1;
							block2_ptr = block2;
						}
						block1_ptr[k * block_size + l] = mat1->values[start_row + k][start_col + l];
						block2_ptr[k * block_size + l] = mat2->values[start_row + k][start_col + l];

					}
				}

				if (i != 0 && j != 0) {

					int dest;
					int coords[] = {i, j};
					MPI_Cart_rank(comm_cart, coords, &dest);

					MPI_Send(block1, bl_size_sq, MPI_FLOAT, dest, 0, comm_cart);
					MPI_Send(block2, bl_size_sq, MPI_FLOAT, dest, 0, comm_cart);

				}

			}
		}

		


		do_multiplication();

		
		//crop result

		//print result
	    
		

		
		

		//deallocation
		matrix_destroy(mat1);
		matrix_destroy(mat2);
		matrix_destroy(product);
		matrix_destroy(org_product);

		free(block1);
		free(block2);
		free(mblock1);
		free(mblock2);


	} else {

		do_multiplication();

	}
	
    


    //finalization
    MPI_Finalize();


}


void do_multiplication() {




	//fetch initial blocks
	float *block1;
	float *block2;

	if (myrank == master_rank) {
		block1 = mblock1;
		block2 = mblock2;
	} else {

		block1 = malloc(bl_size_sq * sizeof(float));
		block2 = malloc(bl_size_sq * sizeof(float));

		MPI_Recv(block1, bl_size_sq, MPI_FLOAT, master_rank, 0, comm_cart, MPI_STATUS_IGNORE);
		MPI_Recv(block1, bl_size_sq, MPI_FLOAT, master_rank, 0, comm_cart, MPI_STATUS_IGNORE);

	}


	
	float *result = malloc(bl_size_sq * sizeof(float));

	int i, j;
	for (i=0; i<block_size; i++) {
		for (j=0; j<block_size; j++) {
			result[i * block_size + j] = 0;
		}
	}

	


	//initial shifts


	//multiply and add to sum in a loop and a half




	//send sum to master process

	//deallocation
	free(block1);
	free(block2);
	free(result);

}