#include <iostream>     
#include <fstream>      
#include <vector>
#include <string>
#include <algorithm>    
#include <iterator>     
#include <boost/tokenizer.hpp>
struct event
{
    public:
   int device;
   char phase;
   float time_stamp;
   int file_num;
};

std::string get_blued_path(int);

std::vector< std::vector <int > > read_csv(std::string file_name)
{
    using namespace std;
    using namespace boost;
    string data(file_name);

    ifstream in(data.c_str());

    typedef tokenizer< escaped_list_separator<char> > Tokenizer;
    vector< string > vec;
    string line;
    
    vector< vector< int > > res;
    
    if (!in.is_open()) 
        return res;

    int header_count = 24;

    while (getline(in, line))
    {   
        if (header_count > 0)
        {
            --header_count;
            continue;
        }
        Tokenizer tok(line);
        vec.assign(tok.begin(), tok.end());
        vector< int > temp;
        for(int i = 0; i < vec.size(); i++)
            temp.push_back(atoi(vec[i].c_str()));
        res.push_back(temp);    
    }
    return res;
}
void extract(struct event events[], float bef_time, float aft_time)
{
    using namespace std;
    // Getting len of events
    int N = sizeof events / sizeof events[0];

    for(int i = 0; i < N; i++)
    {   
        // Getting path of the file
        string path = get_blued_path(events[i].file_num);
        cout << events[i].file_num;

        vector< vector<int > > data, temp;
        data = read_csv(path);

        // Missing data fix
        if (data[0][0] > (events[i].time_stamp - bef_time))
        {
            string bef_path = get_blued_path(events[i].file_num - 1);
            temp = read_csv(bef_path);
            for(int itr = 0; itr < data.size(); itr++)
                temp.push_back(data[itr]);
            data = temp;
        }
        else if(data[0][0] < (events[i].time_stamp + aft_time))
        {
            string aft_path = get_blued_path(events[i].file_num + 1);
            temp = read_csv(aft_path);
            for(int itr = 0; itr < temp.size(); itr++)
                data.push_back(temp[itr]);
        }
        // Finding event
        vector< vector< int > > data_events;
        
        for(int itr = 0; itr < data.size(); itr++)
            if((data[i][0] > (events[i].time_stamp - bef_time)) &&  (data[0][0] < (events[i].time_stamp + aft_time)))
                data_events.push_back(data[i]);
        
        // Index based on phase
        int indx = events[i].phase != 'A' ? 2 : 1;
        
        // Get that column alone
        vector< int > res;
        for(int itr = 0; itr < data_events.size(); itr++)
            res.push_back(data_events[itr][indx]);   
        
        // Write to txt file
        string output_file_name = "final_events/dataset_"+to_string(events[i].file_num)+"/"+to_string(events[i].device)+"_"+events[i].phase+"_"+to_string(events[i].time_stamp)+".txt";
        ofstream output_fp(output_file_name.c_str());

        for(int itr = 0; itr < res.size(); itr++)
        {
			// For no comma in the last element
            if (itr == res.size() - 1)
                {
                    output_fp << res[itr]; 
                    break;
                }
            if (itr < res.size())
                output_fp << res[itr] <<",";
        }        
    }
    
}
