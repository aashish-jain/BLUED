#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <map>
#include <iterator>

#include <unistd.h>
#include "mpi.h"


#define MASTER_PROCESS 0
#define START_END_FILENAME "start_end.csv"
#define DATA_FOLDERS 16
#define DATA_FOLDERS_PATH_FORMAT "../location_001_dataset_%03d/"

using namespace std;

int check_for_start_end_file(int rank, int size)
{
    //check for existing start_end_times else create a new file
    int create_start_end;
    //check if the start end file exists
    fstream start_end_file(START_END_FILENAME, ios_base::in);
    create_start_end = !start_end_file;
    if (create_start_end)
        cout << "\tCreating start end files " << endl;
    return create_start_end;
    
}

float get_start_time(fstream &f)
{
    f.seekg(0,ios_base::beg);
    string line = "";
    for (int count=0 ;getline(f, line) && count<24 ; count++);
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

int get_file_num(char *filename){
    return atoi(filename+20);
}

char *get_full_path(char *dir, char *file){
    string path=string(dir)+string(file);
    // cout<<type()
    char *p =new char[path.size() + 1];
    strcpy(p,path.c_str());
    p[path.size()] = '\0';
    return p;
}

int *get_folders_to_process(int rank, int size, int &folder_num)
{
    //To print the allocated folders propely a small dealy as a function of rank
    // usleep(1000 * rank);
    // cout << rank << " - ";

    folder_num = DATA_FOLDERS / size;
    int max_folder_num;
    int *folders_to_process = new int[folder_num];

    int i;
    for (i = 1; i <= folder_num; i++)
        folders_to_process[i - 1] = rank * folder_num + i;

    //If the number of folders aren't a multiple of number of processes, assign the remaining to other processes
    if (DATA_FOLDERS % size && rank < DATA_FOLDERS % size)
        {
            folders_to_process[i - 1] = rank + 1 + (DATA_FOLDERS / size) * size;
            i++;
        }

    folder_num = i - 1;
    // for (i = 0; i < folder_num; i++)
    //     cout << folders_to_process[i] << ' ';
    // cout << endl;

    return folders_to_process;
}


/* def find_se_time():
    se_time=pd.DataFrame(columns=['start','end'])
    for i in range(len(data_sets)):
        try:
    #         print(data_list[i])
            with open (data_list[i],'rb') as f:
                #skipping the first 24 lines (headers)
                for x in range(24):
                    f.readline()
                #saving the position and reading the first line
                pos,a = f.tell(),float(f.readline().decode("utf-8").split(',')[0])
                #Readoing the last line of the file
                c = f.readline().decode("utf-8")
                f.seek(-80,2)
                b=''
                while True:
                    c=f.readline()
                    if len(c)==0:
                        break
                    b=c
                x=b.decode("utf-8")
                b=float(x.split(',')[0])

                se_time.loc[data_sets[i]]=[a,b]
        except Exception as e:
            print(e)
            print(data_list[i])
    return se_time
*/

int is_data_file(char *file_name){
    return string(file_name).find("ivdata") != string::npos;
}

void create_start_end_log(int *folders_to_process, int folder_num, map<int,float> &start, map<int,float> &end)
{
    for (int i = 0; i < folder_num; i++)
    {
        char *directory = get_path(folders_to_process[i]);
        // cout<<directory<<endl;
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
                if(!is_data_file(file_name))
                    continue;
                file_name = get_full_path(directory,file_name);
                fstream f(file_name,ios_base::in);
                if(!f){
                    cout<<file_name<<" - Error"<<endl;
                    continue;
                }
                start.insert(pair <int, float> (file_num, get_start_time(f)));
                end.insert(pair <int, float> (file_num, get_end_time(f)));
                // printf("%s %f %f\n", file_name, start[file_num], end[file_num]);
            }
            closedir(dir);
        }
        else
            perror("");
    }
}

int main(int argc, char **argv)
{
    int rank, size;

    //start MPI
    MPI_Init(&argc, &argv);
    // get current process id
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //if no arguments then return
    if (argc != 2)
    {
        if (rank == MASTER_PROCESS)
            cout << "Cannot extract events from the dataset if no event-length is specified. Quitting";
        //End process
        MPI_Finalize();
        return 1;
    }

    //parse event length from the cmdline argument
    float event_length = atof(argv[1]);
    int create_start_end;
    if (rank == MASTER_PROCESS)
    {
        cout << "Each event extracted will be of lenght " << event_length << " seconds\n";
        create_start_end = check_for_start_end_file(rank, size);
    }

    MPI_Bcast(&create_start_end, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);

    map <int,float> start,end;
    if (create_start_end)
    {
        //datafolders to process
        int folder_num, *folders_to_process;
        folders_to_process = get_folders_to_process(rank, size, folder_num);
        create_start_end_log(folders_to_process, folder_num, start, end);
    }

    // usleep(10000*rank);
    map <int, float> :: iterator itr;
    for (itr = start.begin(); itr != start.end(); ++itr)
        printf("%04d %6.5f %6.5f\n",itr->first, start[itr->first], end[itr->first]);
    cout << endl;
    //Wait for all the tasks to complete
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank==MASTER_PROCESS)
        cout<<"Done"<<endl;
    //End process
    MPI_Finalize();
    return 1;
}