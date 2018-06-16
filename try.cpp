#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <ctype.h>

#include <mpi.h>

#include <list>
#include <unistd.h>
#include <time.h>
#define MASTER_PROCESS 0
#define NUM_ELEMENTS 10

#define TAG 0
using namespace std;



int main(int argc, char **argv)
{
    int size, rank, event_num;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char file_to_read[100];
    switch(rank){
        case 0: strcpy(file_to_read,"../location_001_dataset_001//location_001_ivdata_001.txt");
                break;
        case 1: strcpy(file_to_read,"../location_001_dataset_001/location_001_ivdata_200.txt");
                break;
        case 2: strcpy(file_to_read,"../location_001_dataset_001/location_001_ivdata_400.txt");
                break;
        case 3: strcpy(file_to_read,"../location_001_dataset_002/location_001_ivdata_600.txt");
                break;
        case 4: strcpy(file_to_read,"../location_001_dataset_002/location_001_ivdata_800.txt");
                break;
    }

    fstream f(file_to_read);
    if(!f)
        cout<<rank<<" Error\n";
    else{
        for(string s;getline(f,s););
    }
    MPI_Finalize();
    return 0;
}