#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>

#include <time.h>
using namespace std;



float get_start_time(fstream &f)
{
    f.seekg(0,ios_base::beg);
}

int main(int argc, char **argv)
{
    time_t t;
    fstream f("../location_001_dataset_001/location_001_eventslist.txt", ios_base::in);
    if(!f){
        cout<<"Unable to read file\n";
        return 0;
    }
    f.close();
    return 0;
}