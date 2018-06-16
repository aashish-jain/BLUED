#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <string.h>
#include <boost/tokenizer.hpp>

#define DATA_FOLDERS_PATH_FORMAT "../location_001_dataset_%03d/"
#define FILE_FORMAT "location_001_ivdata_%03d.txt"

struct event
{
  public:
    int device;
    char phase;
    float time_stamp;
    int file_num;
};

using namespace std;
using namespace boost;


char *get_datafolder(int num)
{
    char *arr = new char[strlen(DATA_FOLDERS_PATH_FORMAT) + 1];
    sprintf(arr, DATA_FOLDERS_PATH_FORMAT, num);
    return arr;
}
string get_file_path(int file_num)
{
    int dataset_num;
    char *x = new char[15];
    strcpy(x, "Hello");
    if (file_num <= 4800)
        dataset_num = (file_num - 1) / 400 + 1;
    else if (file_num <= 5230)
        dataset_num = 13;
    else if (file_num <= 5630)
        dataset_num = 14;
    else if (file_num <= 6030)
        dataset_num = 15;
    else
        dataset_num = 16;
    char *file_name = new char[50], *dir = new char[100];
    strcpy(dir, get_datafolder(dataset_num));
    sprintf(file_name, FILE_FORMAT, file_num);
    strcat(dir, file_name);
    return string(dir);
}

vector<vector<float> > read_csv(string file_name)
{
    string data(file_name);
    fstream file(data.c_str(), ios_base::in);

    typedef tokenizer<escaped_list_separator<char> > Tokenizer;
    vector<string> vec;
    string line;

    vector<vector<float> > res;

    if (!file)
    {
        cout << "File not found!";
        return res;
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
        res.push_back(temp);
    }
    return res;
}

void extract(event *events, int event_num, float bef_time, float aft_time)
{

    for (int i = 0; i <event_num; i++)
    {
        // Getting path of the file
        string path = get_datafolder(events[i].file_num);
        cout << events[i].file_num;

        vector<vector<float> > data, temp;
        data = read_csv(path);

        // Missing data fix
        if (data[0][0] > (events[i].time_stamp - bef_time))
        {
            string bef_path = get_datafolder(events[i].file_num - 1);
            temp = read_csv(bef_path);
            for (int itr = 0; itr < data.size(); ++itr)
                temp.push_back(data[itr]);
            data = temp;
        }
        else if (data[data[0].size()-1][0] < (events[i].time_stamp + aft_time))
        {
            string aft_path = get_datafolder(events[i].file_num + 1);
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
        vector<int> res;
        for (int itr = 0; itr < data_events.size(); ++itr)
            res.push_back(data_events[itr][indx]);

    }
}

int main()
{
    string s("../location_001_dataset_001/location_001_ivdata_001.txt");
    vector <vector<float> > f = read_csv(s);

    return 0;
}