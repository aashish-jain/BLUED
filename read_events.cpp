#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <dirent.h>


#include <string.h>
#include <map>
#include <vector>
#include <iterator>
#include <list>
#include <boost/tokenizer.hpp>

#include<omp.h>
#include <unistd.h>
#include "mpi.h"

#define TAG 0
#define MASTER_PROCESS 0
#define START_FOLDER_NUM 1
#define END_FOLDER_NUM 13
#define STOP_AT 5054
#define DATA_FOLDERS_PATH_FORMAT "../location_001_dataset_%03d/"
#define FILE_FORMAT "location_001_ivdata_%03d.txt"

using namespace std;
using namespace boost;

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

char *get_datafolder(int num)
{
    char *arr = new char[strlen(DATA_FOLDERS_PATH_FORMAT) + 1];
    sprintf(arr, DATA_FOLDERS_PATH_FORMAT, num);
    return arr;
}

int get_file_num(char *filename)
{
    return atoi(filename + 20);
}

char *get_file_path(char *dir, char *file)
{
    string path = string(dir) + string(file);
    char *p = new char[path.size() + 1];
    strcpy(p, path.c_str());
    p[path.size()] = '\0';
    return p;
}

char *get_file_path(int file_num){
    int dataset_num;
    char *x = new char[15];
    strcpy(x,"Hello");
    if(file_num<=4800)
        dataset_num = (file_num-1)/400+1;
    else if(file_num<=5230)
        dataset_num = 13;
    else if(file_num<=5630)
        dataset_num = 14;
    else if(file_num<=6030)
        dataset_num = 15;
    else
        dataset_num = 16; 
    char *file_name = new char[50], *dir= new char[100];
    strcpy(dir,get_datafolder(dataset_num));
    sprintf(file_name, FILE_FORMAT, file_num);
    strcat(dir,file_name);    
    return dir;
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
        char *directory = get_datafolder(i);
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
                file_name = get_file_path(directory, file_name);
                fstream f(file_name, ios_base::in);
                if (!f)
                {
                    cout << file_name << " - Error" << endl;
                    continue;
                }
                float t_start = get_start_time(f), t_end = get_end_time(f);
                file_timestamps temp_fse(t_start, t_end);
                fse.insert(pair<int, file_timestamps>(file_num, temp_fse));
            }
            closedir(dir);
        }
        else
            perror("");
    }
    return fse;
}

void update_event_files(list<event> &eve, map<int, file_timestamps> fse, int &event_num)
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
            event_num++;
            // cout << "!";
            ++event_itr;
        }
        else
            ++fse_itr;
        // cout << endl;
    }
}

event *convert_list_to_array(list<event> e,int &event_num, int size)
{
    list<event>::iterator itr = e.begin();
    itr=e.begin();

    event_num = event_num + ((event_num % size) ? size - event_num % size : 0);
    event *eve_arr = new event[event_num];
    int i;

    for (i = 0; i < event_num && itr!=e.end(); i++, ++itr)
        eve_arr[i] = *itr;
    while(i<event_num)
        eve_arr[i]=event(0,'A',0.0,-1);
    return eve_arr;
}

event *assign_jobs(event *e, int &event_num)
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

    MPI_Bcast(&event_num, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);
    event *recv = new event[event_num];
    MPI_Scatter(e, event_num, mpi_event, recv, event_num, mpi_event, TAG, MPI_COMM_WORLD);
    MPI_Type_free(&mpi_event);

    // usleep(100000*rank);
    // cout<<rank<<' '<<event_num<<endl;
    // for(event *itr=recv;itr!=recv+5;++itr)
    //     itr->print();

    return recv;
}


vector<vector<float> > read_csv(string file_name)
{
    fstream file(file_name.c_str(), ios_base::in);

    typedef tokenizer<escaped_list_separator<char> > Tokenizer;
    vector<string> vec;
    string line;

    vector<vector<float> > result;
    if (!file)
    {
        cout << "File not found!"<<file_name<<endl;
        return result;
    }

    int header_count = 24;
    while (getline(file, line))
    {
        if (header_count > 0)
        {
            --header_count;
            continue;
        }
        Tokenizer tok(line);
        vec.assign(tok.begin(), tok.end());

        vector<float> temp;
        for (int i = 0; i < vec.size(); i++)
            temp.push_back(atof(vec[i].c_str()));
        result.push_back(temp);
    }
    file.close();
    return result;
}

void inline extract_events(event *events, int event_num, float bef_time, float aft_time)
{

    for (int i = 0; i <event_num; i++)
    {
        if(events[i].file_num<0)
            continue;
        // Getting path of the file
        string path(get_file_path(events[i].file_num));
        cout<<path<<endl;
 
        vector<vector<float> > data, temp;
        data = read_csv(path);

        // Missing data fix
        if (data[0][0] > (events[i].time_stamp - bef_time))
        {
            string bef_path = get_file_path(events[i].file_num - 1);
            temp = read_csv(bef_path);
            for (int itr = 0; itr < data.size(); ++itr)
                temp.push_back(data[itr]);
            data = temp;
        }
    
        else if (data[data[0].size()-1][0] < (events[i].time_stamp + aft_time))
        {
            string aft_path = get_file_path(events[i].file_num + 1);
            temp = read_csv(aft_path);
            for (int itr = 0; itr < temp.size(); ++itr)
                data.push_back(temp[itr]);
        }
        // Finding event
        vector<vector<float> > data_events;

        for (int itr = 0; itr < data.size(); ++itr)
            if ((data[itr][0] > (events[i].time_stamp - bef_time)) && (data[itr][0] < (events[i].time_stamp + aft_time)))
                data_events.push_back(data[itr]);

        // Index based on phase
        int indx = (events[i].phase == 'A') ? 1 : 2;

        // Get that column alone
        vector<int> result;
        for (int itr = 0; itr < data_events.size(); ++itr)
            result.push_back(data_events[itr][indx]);
        
                // Write to txt file

        char output_file_name[100];
        sprintf(output_file_name,"events/%d_%c_%f.txt",events[i].device,events[i].phase,events[i].time_stamp);
        fstream output_fp(output_file_name,ios_base::out);
        for(int itr = 0; itr < result.size(); itr++)
        {
			// For no comma in the last element
            if (itr == result.size() - 1)
                {
                    output_fp << result[itr]; 
                    break;
                }
            if (itr < result.size())
                output_fp << result[itr] <<",";
        }    
        output_fp.close();
    }
}

int main(int argc, char **argv)
{
    int rank, size;

    // event *events;
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
    int event_num=0;
    event *events;
    if (rank == MASTER_PROCESS)
    {
        cout << "Each event extracted will be of length " << after_event_length + before_event_length << " seconds\n";

        // Get the starting and ending times
        map<int, file_timestamps> file_start_end = create_start_end_log();
        list<event> event_list = events_list_parser();

        // Changes from here
        update_event_files(event_list, file_start_end, event_num);
        // print(event_list);

        //Adds dummy events to round off the jobs for easy access
        events = convert_list_to_array(event_list, event_num, size);

        //sanity-check
        // for (int i = 0; i < event_num; i++)
        //     {cout<<i<< ' '<<events[i].file_num<<' ';
        //     events[i].print();}
        // cout<<event_num<<endl;
        //To be distributed amongst other processes
        event_num /= size;
    }
    //Changes below
    //Error here!!!!!

    event *recieved=assign_jobs(events, event_num);

    cout<<"Rank = "<<rank<<" #Events = "<<event_num<<endl;

    // for(event *itr=recieved;itr!=recieved+event_num;++itr)
    //     itr->print();

    extract_events(recieved, event_num, before_event_length, after_event_length);
   
    if (rank == MASTER_PROCESS)
        cout << "Done" << endl;


    //End process
    MPI_Finalize();
    return 0;
}