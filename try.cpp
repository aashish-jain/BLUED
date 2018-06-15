#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <ctype.h>

#include <list>
#include <time.h>
using namespace std;

struct event{
    public:
    int device;
    int phase;
    float time_stamp;

    void print(){
        cout<<device<< ' '<<phase<<' '<<time_stamp<<endl;
    }
};

list<event> events_list_parser(){
    list<event> l;
    fstream f("events_list.csv", ios_base::in);
    if(!f){
        cout<<"Unable to read file\n";
        return l;
    }
    string line;
    getline(f,line);
    event event_obj;
    for (int count=0;getline(f,line) && count<10;count++){
        char buff[15];
        //Index (DUMMY)
        int i=0,j=0;
        for(j=0;i<line.size() && line[i]!=',';i++,j++)
            buff[j]=line[i];
        buff[j]='\0';
        i++;
        //device
        for(j=0;i<line.size() && line[i]!=',';i++,j++)
            buff[j]=line[i];
        buff[j]='\0';
        i++;
        event_obj.device = atoi(buff);
        //Timestamp
        for(j=0;i<line.size() && line[i]!=',';i++,j++)
            buff[j]=line[i];
        buff[j]='\0';
        i++;
        event_obj.time_stamp = atof(buff);
        //phase
        for(j=0;i<line.size() && line[i]!=',';i++,j++)
            buff[j]=line[i];
        buff[j]='\0';
        i++;
        event_obj.phase = buff[j-1];
        l.push_back(event_obj);
    }
    f.close();
    return l;
}


void showlist(list <event> g)
{
    list <event> :: iterator it;
    for(it = g.begin(); it != g.end(); ++it)
        it->print();
}
int main(int argc, char **argv)
{
    list <event> l=events_list_parser();
    showlist(l);
    return 0;
}

