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

struct event
{
  public:
    int device;
    char phase;
    float time_stamp;
    int file_num;

    event(int d, char p, float t, int f)
    {
        device = d;
        phase = p;
        time_stamp = t;
        file_num = f;
    }
    void init(int d, char p, float t, int f)
    {
        device = d;
        phase = p;
        time_stamp = t;
        file_num = f;
    }
    event()
    {
        file_num = -1;
    }
    void print()
    {
        if (file_num != -1)
            cout << device << ' ' << phase << ' ' << time_stamp << ' ' << file_num << endl;
    }
};


event *assign_jobs_working(int rank, int size, int &event_num)
{
    const int n_items = 4;
    int block_len[n_items] = {1, 1, 1, 1};
    MPI_Datatype types[n_items] = {MPI_INT, MPI_CHAR, MPI_FLOAT, MPI_INT};
    MPI_Datatype mpi_event;
    MPI_Aint offsets[n_items];

    offsets[0] = offsetof(event, device);
    offsets[1] = offsetof(event, phase);
    offsets[2] = offsetof(event, time_stamp);
    offsets[3] = offsetof(event, file_num);

    MPI_Type_create_struct(4, block_len, offsets, types, &mpi_event);
    MPI_Type_commit(&mpi_event);

    event *e;

    //rounding off to have equal number of jobs so that everyone is assigned a job
    if (rank == MASTER_PROCESS)
    {
        e = new event[event_num];
        for (int i = 0; i < event_num; i++)
            e[i].init(i * 10, 'A' + i, i * 100, i);
        event_num /= size;
    }

    MPI_Bcast(&event_num, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);
    event *recv = new event[event_num];

    MPI_Scatter(e, event_num, mpi_event, recv, event_num, mpi_event, TAG, MPI_COMM_WORLD);
    MPI_Type_free(&mpi_event);
    return recv;
}

event *convert_list_to_array(list<event> e,int number_of_events)
{
    list<event>::iterator itr = e.begin();
    event *eve_arr = new event[number_of_events];
    itr=e.begin();
    for (int i = 0; i < number_of_events; i++, ++itr)
        eve_arr[i] = *itr;
    return eve_arr;
}

int main(int argc, char **argv)
{
    int size, rank, event_num;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == MASTER_PROCESS)
    {
        list<event> el;
        for (int i = 0; i < 10; i++)
            el.push_back(event(i * 10, 'A' + i, i * 100, i));

        int num_events=el.size();
        event *e = convert_list_to_array(el, num_events);

        for (int i = 0; i < 10; i++)
            e[i].print();
        event_num = NUM_ELEMENTS + ((NUM_ELEMENTS % size) ? size - NUM_ELEMENTS % size : 0);
    }
 
    event *recieved=assign_jobs_working(rank,size,event_num);
    // usleep(10000*rank);
    // cout<<rank<<' '<<event_num<<endl;
    // for(event *itr=recieved;itr!=recieved+event_num;++itr)
    //     itr->print();

    MPI_Finalize();
    return 0;
}