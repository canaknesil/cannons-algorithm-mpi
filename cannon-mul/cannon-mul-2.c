#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "util/matrix.h"



float *block1;
float *block2;
float *sum;


int main(int argc, char *argv[]) {


	//initialisation
	MPI_Init(NULL, NULL);

	//getting information
	int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);


	//getting information
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

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


	//calculate extended dimentions
	int row1 = org_row1 + (org_row1 % block_size == 0 ? 0 : block_size - org_row1 % block_size);
	int col1 = org_col1 + (org_col1 % block_size == 0 ? 0 : block_size - org_col1 % block_size);
	int row2 = col1;
	int col2 = org_col2 + (org_col2 % block_size == 0 ? 0 : block_size - org_col2 % block_size);

	int bl_size_sq = block_size * block_size;
	int nblock_row = row1 / block_size;
	int nblock_col = col2 / block_size;



	//checking number of process
	int dims[] = {row1 / block_size, col2 / block_size};

	int expected_np = dims[0] * dims[1];
	if (world_size != expected_np) {
		if (myrank == master_rank) printf("Incorrect number of process. There must be %d process.\nQuitting...\n", expected_np);
		MPI_Finalize();
		exit(1);
	}




	//create cartesian topology
    int periods[] = {1, 1}; //all dimentions have wraparound connections
    MPI_Comm comm_cart;

    MPI_Cart_create(MPI_COMM_WORLD, /*ndims*/ 2, dims, periods, /*reorder*/ 1, &comm_cart);

    //getting new rank information after reordering
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);





	//allocation for all the processes
	block1 = malloc(bl_size_sq * sizeof(float));
	block2 = malloc(bl_size_sq * sizeof(float));
	sum = malloc(bl_size_sq * sizeof(float));

	//allocation for master process
	Matrix *mat1;
	Matrix *mat2;
	Matrix *product;

	int i, j, k, l;

	if (myrank == master_rank) {

		//srand(time(NULL));
		if (see_progress && myrank == master_rank) {
			printf("Creating matrices... ");
		}

		mat1 = matrix_create_random(row1, col1);
		mat2 = matrix_create_random(row2, col2);
		product = matrix_create(row1, col2);

		//fill extra space with zeros
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

		if (see_progress && myrank == master_rank) {
			printf("Done.\n");
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


	}


	double begin;
	double end;
	double time_spent;
	

	if (see_progress && myrank == master_rank) {
		printf("Setting initial locations of submatrices... ");
	}

	//send appropriate blocks to processes
	if (myrank == master_rank) {

		float *buf1 = malloc(bl_size_sq * sizeof(float));
		float *buf2 = malloc(bl_size_sq * sizeof(float));

		for (i=0; i<nblock_row; i++) {
			for (j=0; j<nblock_col; j++) {
				//every block

				int start_row = i * block_size;
				int start_col = j * block_size;

				float *block1_ptr;
				float *block2_ptr;
				if (i == 0 && j == 0) {
					block1_ptr = block1;
					block2_ptr = block2;
				} else {
					block1_ptr = buf1;
					block2_ptr = buf2;
				}

				for (k=0; k<block_size; k++) {
					for (l=0; l<block_size; l++) {

						block1_ptr[k * block_size + l] = mat1->values[start_row + k][start_col + l];
						block2_ptr[k * block_size + l] = mat2->values[start_row + k][start_col + l];

					}
				}
				
				if (i != 0 || j != 0) {

					int dest;
					int coords[] = {i, j};
					MPI_Cart_rank(comm_cart, coords, &dest);
					
					MPI_Send(buf1, bl_size_sq, MPI_FLOAT, dest, 0, comm_cart);
					MPI_Send(buf2, bl_size_sq, MPI_FLOAT, dest, 0, comm_cart);

				}

			}
		}

		free(buf1);
		free(buf2);

	} else {

		MPI_Recv(block1, bl_size_sq, MPI_FLOAT, master_rank, 0, comm_cart, MPI_STATUS_IGNORE);
		MPI_Recv(block2, bl_size_sq, MPI_FLOAT, master_rank, 0, comm_cart, MPI_STATUS_IGNORE);

	}

	if (see_progress && myrank == master_rank) {
		printf("Done.\n");
	}
	

	if (see_progress && myrank == master_rank) {
		printf("Making initial shiftings... ");
	}


	//do the multiplication
	begin = MPI_Wtime();




	//initial shifts

	//along x-axix
	int coords[2];
	MPI_Cart_coords(comm_cart, myrank, 2, coords);

	int s_step =  -coords[0];
	int rank_source, rank_dest;
	MPI_Cart_shift(comm_cart, /*along x-axis to right*/ 1, s_step, &rank_source, &rank_dest);

	MPI_Sendrecv_replace(block1, bl_size_sq, MPI_FLOAT, rank_dest, 0, rank_source, 0, comm_cart, MPI_STATUS_IGNORE);

	//along y-axix
	s_step = -coords[1];
	MPI_Cart_shift(comm_cart, /*along y-axis to right*/ 0, s_step, &rank_source, &rank_dest);

	MPI_Sendrecv_replace(block2, bl_size_sq, MPI_FLOAT, rank_dest, 0, rank_source, 0, comm_cart, MPI_STATUS_IGNORE);



	if (see_progress && myrank == master_rank) {
		printf("Done.\n");
	}


	if (see_progress && myrank == master_rank) {
		printf("Multiplying...\n");
		printf("\r%.0f %%", (float) 0);
	}



	//multiply and add to sum in a loop and a half
	float *inter_mul = malloc(bl_size_sq * sizeof(float));

	//initialize sum as zeros
	for (i=0; i<block_size; i++) {
		for (j=0; j<block_size; j++) {
			sum[i * block_size + j] = 0;
		}
	}



	//multiply and add
	matrix_mul_serial_cont(block1, block2, inter_mul, block_size, block_size, block_size);
	matrix_add_cont(sum, inter_mul, block_size, block_size);


	if (see_progress && myrank == master_rank) {
		printf("\r%.0f %%", (float) 1 / nblock_row * 100);
	}


	for (i=0; i < nblock_row - 1; i++) {

		//shift
		MPI_Cart_shift(comm_cart, /*along x-axis to right*/ 1, -1, &rank_source, &rank_dest);
		MPI_Sendrecv_replace(block1, bl_size_sq, MPI_FLOAT, rank_dest, 0, rank_source, 0, comm_cart, MPI_STATUS_IGNORE);

		MPI_Cart_shift(comm_cart, /*along y-axis to right*/ 0, -1, &rank_source, &rank_dest);
		MPI_Sendrecv_replace(block2, bl_size_sq, MPI_FLOAT, rank_dest, 0, rank_source, 0, comm_cart, MPI_STATUS_IGNORE);

		//multiply and add
		matrix_mul_serial_cont(block1, block2, inter_mul, block_size, block_size, block_size);
		matrix_add_cont(sum, inter_mul, block_size, block_size);

		if (see_progress && myrank == master_rank) {
			printf("\r%.0f %%", (float) i+2 / nblock_row * 100);
		}

	}


	free(inter_mul);

	if (see_progress && myrank == master_rank) {
		printf("\n");
	}


	if (see_progress && myrank == master_rank) {
		printf("Gethering result... ");
	}


	//send sum to master process
	if (myrank == master_rank) {

		float *sum_buf = malloc(bl_size_sq * sizeof(float));

		for (i=0; i<nblock_row; i++) {
			for (j=0; j<nblock_col; j++) {
				//every block

				float *sum_ptr;
				if (i == 0 && j == 0) {

					sum_ptr = sum;

				} else {

					sum_ptr = sum_buf;

					int source_rank;
					int coords[2] = {i, j};
					MPI_Cart_rank(comm_cart, coords, &source_rank);

					MPI_Recv(sum_ptr, bl_size_sq, MPI_FLOAT, source_rank, 0, comm_cart, MPI_STATUS_IGNORE);

				}

				//combine result
				int start_row = i * block_size;
				int start_col = j * block_size;

				for (k=0; k<block_size; k++) {
					for (l=0; l<block_size; l++) {

						product->values[start_row + k][start_col + l] = sum_ptr[k * block_size + l];
						
					}
				}
				

			}
		}

		free(sum_buf);

	} else {

		MPI_Send(sum, bl_size_sq, MPI_FLOAT, master_rank, 0, comm_cart);

	}

	if (see_progress && myrank == master_rank) {
		printf("Done.\n");
	}


	end = MPI_Wtime();
	time_spent = end - begin;

	if (myrank == master_rank) {
		printf("%f\n", time_spent);
	}


	//print result
	if (print_matrices && myrank == master_rank) {

		printf("Product:\n");
		matrix_print(product);
		printf("\n");

	}




    
	//deallocation to all the processes
    free(block1);
	free(block2);
	free(sum);

	//deallocation for master process
	if (myrank == master_rank) {

		matrix_destroy(mat1);
		matrix_destroy(mat2);
		matrix_destroy(product);

	}
	


    //finalization
    MPI_Finalize();


}

