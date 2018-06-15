#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <dirent.h>

#include <string.h>
#include <map>
#include <iterator>
#include <list>

#include <unistd.h>
#include "mpi.h"

#define MASTER_PROCESS 0
#define START_FOLDER_NUM 1
#define END_FOLDER_NUM 13 
#define STOP_AT 5054
#define DATA_FOLDERS_PATH_FORMAT "../location_001_dataset_%03d/"

using namespace std;

struct file_timestamps
{
  public:
    float start, end;
    file_timestamps()
    {
        start = end = 0;
    }

    file_timestamps(float s, float e)
    {
        start = s;
        end = e;
    }

    void print()
    {
        cout << start << ' ' << end << endl;
    }
};

struct event
{
  public:
    int device;
    char phase;
    float time_stamp;
    int file_num;

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

void print(list<event> l)
{
    list<event>::iterator itr;
    for (itr = l.begin(); itr != l.end(); ++itr)
        itr->print();
}

void print(map<int, file_timestamps> file_start_end)
{
    for (map<int, file_timestamps>::iterator itr = file_start_end.begin(); itr != file_start_end.end(); ++itr)
        itr->second.print();
}

float get_start_time(fstream &f)
{
    f.seekg(0, ios_base::beg);
    string line = "";
    for (int count = 0; getline(f, line) && count < 24; count++)
        ;
    return atof(line.c_str());
    //do something with the line
}

float get_end_time(fstream &f)
{
    //Go backwards
    f.seekg(-100, ios_base::end);
    string line = "", prv_line = "";
    for (; getline(f, line);)
        prv_line = line;
    return atof(prv_line.c_str());
}

char *get_path(int num)
{
    char *arr = new char[strlen(DATA_FOLDERS_PATH_FORMAT) + 1];
    sprintf(arr, DATA_FOLDERS_PATH_FORMAT, num);
    return arr;
}

int get_file_num(char *filename)
{
    return atoi(filename + 20);
}

char *get_full_path(char *dir, char *file)
{
    string path = string(dir) + string(file);
    // cout<<type()
    char *p = new char[path.size() + 1];
    strcpy(p, path.c_str());
    p[path.size()] = '\0';
    return p;
}

list<event> events_list_parser()
{
    list<event> l;
    fstream f("events_list.csv", ios_base::in);
    if (!f)
    {
        cout << "Unable to read file\n";
        return l;
    }
    string line;
    getline(f, line);
    event event_obj;
    while (getline(f, line))
    {
        char buff[15];
        //Index (DUMMY)
        int i = 0, j = 0;
        for (j = 0; i < line.size() && line[i] != ','; i++, j++)
            buff[j] = line[i];
        buff[j] = '\0';
        i++;
        //device
        for (j = 0; i < line.size() && line[i] != ','; i++, j++)
            buff[j] = line[i];
        buff[j] = '\0';
        i++;
        event_obj.device = atoi(buff);
        //Timestamp
        for (j = 0; i < line.size() && line[i] != ','; i++, j++)
            buff[j] = line[i];
        buff[j] = '\0';
        i++;
        event_obj.time_stamp = atof(buff);
        //phase
        for (j = 0; i < line.size() && line[i] != ','; i++, j++)
            buff[j] = line[i];
        buff[j] = '\0';
        i++;
        event_obj.phase = buff[j - 1];
        l.push_back(event_obj);
    }
    f.close();
    return l;
}

int is_data_file(char *file_name)
{
    return string(file_name).find("ivdata") != string::npos;
}

map<int, file_timestamps> create_start_end_log()
{
    map<int, file_timestamps> fse;
    for (int i = START_FOLDER_NUM; i <= END_FOLDER_NUM; i++)
    {
        char *directory = get_path(i);
        DIR *dir;
        struct dirent *ent;
        char *file_name;
        if ((dir = opendir(directory)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                file_name = ent->d_name;
                //If not a valid datafile
                int file_num = get_file_num(file_name);
                if (!is_data_file(file_name) || file_num > STOP_AT)
                    continue;
                file_name = get_full_path(directory, file_name);
                fstream f(file_name, ios_base::in);
                if (!f)
                {
                    cout << file_name << " - Error" << endl;
                    continue;
                }
                float t_start=get_start_time(f), t_end=get_end_time(f);
                file_timestamps temp_fse(t_start, t_end);
                fse.insert(pair<int, file_timestamps>(file_num, temp_fse));
            }
            closedir(dir);
        }
        else
            perror("");
    }
    // print(fse);
    return fse;
}

void update_event_files(list<event> &eve, map<int, file_timestamps> fse)
{
    list<event>::iterator event_itr = eve.begin();
    map<int, file_timestamps>::iterator fse_itr = fse.begin();
    while (event_itr != eve.end() && fse_itr != fse.end())
    {
        file_timestamps &temp_fse = fse_itr->second;
        // cout << temp_fse.start << ' ' << event_itr->time_stamp << ' ' << temp_fse.end << ' ';
        if (temp_fse.start < event_itr->time_stamp && event_itr->time_stamp < temp_fse.end)
        {
            event_itr->file_num = fse_itr->first;
            // cout << "!";
            ++event_itr;
        }
        else
            ++fse_itr;
        // cout << endl;
    }
}

int main(int argc, char **argv)
{
    int rank, size;
    list<event> event_list;

    //start MPI
    MPI_Init(&argc, &argv);
    // get current process id
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //if no arguments then return
    if (argc != 3)
    {
        if (rank == MASTER_PROCESS)
            cout << "Cannot extract events from the dataset if no event-length is specified. Quitting\n";
        //End process
        MPI_Finalize();
        return 1;
    }

    //parse event length from the cmdline argument
    float before_event_length = atof(argv[1]), after_event_length = atof(argv[2]);
    if (rank == MASTER_PROCESS)
    {
        cout << "Each event extracted will be of lenght " << after_event_length + before_event_length << " seconds\n";

        //Get the starting and ending times
        map<int, file_timestamps> file_start_end = create_start_end_log();


        event_list = events_list_parser();

        update_event_files(event_list, file_start_end);
        print(event_list);
    }

    //Wait for all the tasks to complete
    MPI_Barrier(MPI_COMM_WORLD);

    // extract_events();

    if (rank == MASTER_PROCESS)
        cout << "Done" << endl;

    //End process
    MPI_Finalize();
    return 1;
}