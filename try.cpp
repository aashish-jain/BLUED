#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <ctype.h>

#include<mpi.h>

#include <list>
#include <time.h>
#define MASTER_PROCESS 0
#define NUME 10
using namespace std;



int main(int argc, char **argv) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int *globaldata=NULL;
    int *localdata;
    int nume;

    if (rank == 0) {
        globaldata = new int[size*NUME];
        for (int i=0; i<size*NUME; i++)
            globaldata[i] = 2*i+1;

        printf("Processor %d has data: **", rank);
        for (int i=0; i<size*NUME; i++)
            printf("%d ", globaldata[i]);
        printf("\n");
        nume=NUME;
    }

    MPI_Bcast(&nume,1,MPI_INT,MASTER_PROCESS, MPI_COMM_WORLD);
    printf("Processor %d has nume %d\n", rank, nume);
    MPI_Scatter(globaldata, nume, MPI_INT, localdata, nume, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Processor %d has data ", rank);
    for(int i=0;i<nume;i++) 
        cout<<localdata[i]<<' ';
    cout<<endl;
    // localdata *= 3;
    // printf("Processor %d doubling the data, now has %d\n", rank, localdata);

    // MPI_Gather(&localdata, 1, MPI_INT, globaldata, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // if (rank == 0) {
    //     printf("Processor %d has data: ", rank);
    //     for (int i=0; i<size; i++)
    //         printf("%d ", globaldata[i]);
    //     printf("\n");
    // }

    if (rank == 0)
        free(globaldata);

    MPI_Finalize();
    return 0;
}