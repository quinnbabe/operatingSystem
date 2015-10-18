/*
 Title:Operating System Scheduler
 Description: Discrete Event Simulation (DES).The system progresses in time through state transitions
 and by progressing time discretely between the events.
 The basic process is firstly process blocks for input, secondly scheduler picks another process,
 thirdly scheduler picks the process and finally input becomes available
 Simplification: integer time, random file read in, IOs independent
 Four scheduling algorithms to be simulated: FCFS, RoundRobin, LCFS and SJF
 Author:Ying Qu
 Date: March 11th, 2014
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <list>

#define BLOCKED 1
#define READY 2
#define RUNNING 3
#define NOTIN -9999
#define DONE 9999

using namespace std;

int *randArray;//random array
int rfSize;//random file size
int processSize=0;//the size of the processes
int timer=0;//to record time
int ofs=0;//the index of random array to create random value
int CPUsum=0;
int IOsum=0;
int turnAroundSum=0;
int CPUWaitSum=0;

int myrandom(int burst){
    int value = 1+(randArray[ofs]%burst);
    ofs++;
    if (ofs>=rfSize){
        ofs=0;
    }
    return value;
}

struct process{
    int AT;// arrival time
    int TC;//total CPU time
    int CB;//CPU burst
    int IO;//IO burst
    int FT;//finishing time
    int TT;//turnaround time (finishing time - AT)
    int IT;//I/O time (time in blocked state)
    int CW;// CPU waiting time (time in ready state)
    int lefttime;//the remaining to process
    int CB_burst;
    int IO_burst;
    int ID;
    int label;//to record the order of the sequence.
    /*The order at which they are presented to the system is based on the order they appear in the file when multiple processes have the same arrival times
     */
    
    bool store;
    int state;
    
};

process pArray[200];


bool comparePro(const process &x, const process &y){
    return(x.label<y.label);
};//to compare two process and to determine whose label is smaller

bool cmpArray(const process &x, const process &y){
    return(x.lefttime<y.lefttime);
};//to compare two process and to determine whose lefttime is smaller

//Object oriented programming
//Implement the scheduler as "objects" without replicating the event simulation infrastructure for each case
//the basic scheduler
class sch
{   public:
    char name[20];
    bool vvRecord;
    bool CPUmark;
    bool IOmark;
    bool run;
    int runningtime;
    int quantum;
    char tempArray[20];
    
    virtual void quantumTake (char* sNum){
        
    }//only be used in the RoundRobin scheduler
    
    //the ready queue in the order of their occurrence in the input file
    list<int> line;
    list<process> pendingLine;
    
    virtual void update(){}
    
    sch(){}
    sch(char* sNum, bool vv){
        strcpy(name,"");
        vvRecord = vv;
        quantum = 0;
    }
    
    void simulate(char *sNum){
        
        int countNum=0;
        int a;
        quantumTake(sNum);
        runningtime=0;
        //cout<<quantum<<endl;
        run=true;
        
        
        while(1){
            countNum=0;
            pendingLine.clear();//to clear the process pendingline
            CPUmark =false;
            IOmark =false;
            
            //update states
            for(int i= 0;i<processSize;i++){
                
                if(pArray[i].state==DONE){
                    countNum++;
                }
                else if(pArray[i].state==RUNNING){
                    if (pArray[i].lefttime==0){
                        pArray[i].state=DONE;
                        pArray[i].FT= timer;
                        pArray[i].TT= pArray[i].FT -pArray[i].AT;//turnaroundtime =finishing time - arrival time
                        run =true;
                        runningtime=0;
                        turnAroundSum=pArray[i].TT+turnAroundSum;
                        CPUWaitSum=pArray[i].CW+CPUWaitSum;
                        //cout<<timer<<": "<<i<<"RUNNING->DONE"<<endl;
                        line.pop_front();
                        pArray[i].label=timer;
                    }
                    else if (pArray[i].CB_burst==0){
                        pArray[i].state=BLOCKED;
                        //cout<<"!"<<endl;
                        run =true;
                        runningtime=0;
                        pArray[i].label =timer;
                        pArray[i].IO_burst =myrandom(pArray[i].IO);//call random function for transition 1
                        //cout<<timer<<": "<<i<<"RUNNING->BLOCKED IB="<<pArray[i].IO_burst<<endl;
                        line.pop_front();
                        pArray[i].label=timer;
                    }
                }
                else if(pArray[i].state==BLOCKED){
                    if(pArray[i].IO_burst==0){
                        pArray[i].state=READY;
                        pendingLine.push_back(pArray[i]);
                        //cout<<timer<<": "<<i<<"BLOCKED->READY"<<endl;
                        pArray[i].label=timer;
                    }
                }
            }
            //for loop end, traverse end
            if(countNum==processSize){
                timer--;
                break;
            }
            
            update();
            if(!line.empty()){
                a=line.front();
                if(pArray[a].state!=RUNNING){
                    pArray[a].state=RUNNING;
                    run =false;
                    if(pArray[a].CB_burst==0){
                        pArray[a].CB_burst = myrandom(pArray[a].CB);//call random function for transition 3
                        if (pArray[a].CB_burst>pArray[a].lefttime){pArray[a].CB_burst=pArray[a].lefttime;}
                    }
                    //cout<<timer<<": "<<a<<"READY->RUNNING CB="<<pArray[a].CB_burst<<endl;
                    pArray[a].label=timer;
                }
                
            }
            
            timer++;
            //upate attribute
            for(int i= 0;i<processSize;i++){
                if(pArray[i].state==RUNNING){
                    pArray[i].lefttime =pArray[i].lefttime-1;
                    pArray[i].CB_burst =pArray[i].CB_burst-1;
                    runningtime =runningtime +1;
                    CPUmark =true;
                }
                else if(pArray[i].state==READY){
                    pArray[i].CW= pArray[i].CW+1;
                }
                else if(pArray[i].state==BLOCKED){
                    pArray[i].IT= pArray[i].IT+1;
                    pArray[i].IO_burst =pArray[i].IO_burst-1;
                    IOmark =true;
                }
            }
            if(CPUmark==true){CPUsum++;}
            if(IOmark==true){IOsum++;}
        }
        printf("%s",name);
        if(quantum!=0){
            printf(" %d\n",quantum);
        }
        else{
            printf("\n");
        }
    }
};

//FCFS scheduler
class fcfs_sch : public sch
{   public:
    
    void quantumTake (char* sNum){}
    
    fcfs_sch(char* sNum, bool vv){
        strcpy(name,"FCFS");
        vvRecord = vv;
        quantum = 0;
    }
    
    void update(){
        for(int i=0; i<processSize;i++){
            if (pArray[i].AT==timer){
                pArray[i].state= READY;
                line.push_back(i);
                pArray[i].label=timer;
                
                //cout<<timer<<": "<<i<<"FILE->READY"<<endl;
            }
        }
        //updating line
        pendingLine.sort(comparePro);
        while(!pendingLine.empty()){
            line.push_back(pendingLine.front().ID);
            pendingLine.pop_front();
        }
        
    }
};

//RoundRobin scheduler
//only regenerate a new CPU burst when current one has expired
class rr_sch : public sch
{   public:
    
    //to take the number out except the first letter 'R'
    void quantumTake (char* sNum){
        strcpy(tempArray,"");
        for(int i=1;;i++){
            tempArray[i-1]= sNum[i];
            if(sNum[i]==0){
                break;
            }
        }
        quantum =atoi(tempArray);
    }
    
    rr_sch(char* sNum, bool vv){
        strcpy(name,"RR");
        vvRecord = vv;
        quantum = 0;
    }
    
    void update(){
        for(int i=0; i<processSize;i++){
            if (pArray[i].AT==timer){
                pArray[i].state= READY;
                line.push_back(i);
                pArray[i].label=timer;
                //cout<<timer<<": "<<i<<"FILE->READY"<<endl;
            }
        }
        //updating line
        pendingLine.sort(comparePro);
        while(!pendingLine.empty()){
            line.push_back(pendingLine.front().ID);
            pendingLine.pop_front();
        }
        if (runningtime == quantum){
            int l;
            l=line.front();
            if(pArray[l].state == RUNNING){
                line.push_back(l);
                line.pop_front();
                pArray[l].state = READY;
                pArray[l].label=timer;
                //cout<<timer<<": "<<l<<"RUNNING->READY"<<endl;
                runningtime =0;
            }
        }
        
    }
};

//LCFS scheduler
class lcfs_sch : public sch
{   public:
    
    void quantumTake (char* sNum){}
    
    lcfs_sch(char* sNum, bool vv){
        strcpy(name,"LCFS");
        vvRecord = vv;
        quantum = 0;
    }
    
    void update(){
        int l;
        bool lineBlank =false;
        
        for(int i=0; i<processSize;i++){
            if (pArray[i].AT==timer){
                pArray[i].state= READY;
                if(!line.empty()){
                    l=line.front();
                    if(pArray[l].state==RUNNING){
                        line.pop_front();
                        lineBlank=true;
                        
                     }
                }
                line.push_front(i);
                if(lineBlank==true){
                    line.push_front(l);
                    lineBlank=false;
                }
                pArray[i].label=timer;
                //cout<<timer<<": "<<i<<"FILE->READY"<<endl;
            }
        }
        //updating line
        pendingLine.sort(comparePro);
        
        while(!pendingLine.empty()){
            if(!line.empty()){
                l=line.front();
                if(pArray[l].state==RUNNING){
                    line.pop_front();
                    lineBlank=true;
                    
                }
            }
            line.push_front(pendingLine.front().ID);
            if(lineBlank==true){
                line.push_front(l);
                lineBlank=false;
            }
            pendingLine.pop_front();
        }
    }
};

//SJF scheduler
class sjf_sch : public sch
{   public:
    
    void quantumTake (char* sNum){}
    
    sjf_sch(char* sNum, bool vv){
        strcpy(name,"SJF");
        vvRecord = vv;
        quantum = 0;
    }
    
    void update(){
        for(int i=0; i<processSize;i++){
            if (pArray[i].AT==timer){
                pArray[i].state= READY;
                line.push_back(i);
                pArray[i].label=timer;
                //cout<<timer<<": "<<i<<"FILE->READY"<<endl;
            }
        }
        //updating line
        while(!pendingLine.empty()){
            line.push_back(pendingLine.front().ID);
            pendingLine.pop_front();
        }
        if(run==true){
            pendingLine.clear();
            int l;
            while(!line.empty()){
                l=line.front();
                pendingLine.push_back(pArray[l]);
                line.pop_front();
            }
            pendingLine.sort(cmpArray);
            //schedule is based on the shorest remaining execution time, not shorest CPU burst
            while(!pendingLine.empty()){
                line.push_back(pendingLine.front().ID);
                pendingLine.pop_front();
            }
            
        }
    }
};

int main( int argc, char *argv[]){
    //to read in the input file and random file
    FILE *iFile;
	FILE *rFile;
    char *sNum = NULL;
    char *inputFile = NULL;
    char *randomFile = NULL;
    
    int opt;
    int offset=1;
    bool vv=false;//to determin whether it has the attribute of v
    
    double cpuUtiliaztion;
    double ioUtilization;
    double avgTT;
    double avgCW;
    double proNum;
    
    while((opt = getopt( argc, argv, "vs:"))!=-1 ){
        switch (opt) {
            case 's':
                sNum = optarg;
                offset++;
                break;
            case 'v':
                vv = true;
                offset++;
                break;
            case '?':
                printf("Error!");
                return 0;
            default:
                return 0;
        }
    }
    
    inputFile = argv[offset++];//array
    randomFile = argv[offset];
    
    //inputFile = "/Users/Quinnbabe/Desktop/lab2/lab2/input1";
    //randomFile = "/Users/Quinnbabe/Desktop/lab2/lab2/rfile";
    //sNum="L";//FLRS
    
    //openfile
    iFile=fopen(inputFile, "r");
    rFile=fopen(randomFile, "r");
    
    fscanf(rFile,"%d\n", &rfSize);// to get the first value of the random file to creat the array
    randArray = new int[rfSize];
    
    for(int i=0;i<rfSize;i++){
        fscanf(rFile,"%d\n",&randArray[i]);
        
    }
    
    process t;//temp variable
    while(!feof(iFile)){//end of file
        fscanf(iFile, "%d %d %d %d\n", &t.AT,&t.TC,&t.CB,&t.IO);
        t.FT =0;
        t.TT =0;
        t.IT =0;
        t.CW =0;
        t.lefttime =t.TC;
        t.CB_burst =0;
        t.IO_burst =0;
        t.ID =processSize;
        t.store =false;
        t.state = NOTIN;
        pArray[processSize]=t;
        processSize++;
        
        
    }
    
    //printf("%d\n",array[1]);
    //printf("%d\n",pArray[1].AT);
    
    //to make an instance of FCFS to simulate
    fcfs_sch f_instance(sNum,vv);
    if(sNum[0]=='F'){
        f_instance.simulate(sNum);
    }
    //to make an instance of RoundRobin to simulate
    rr_sch r_instance(sNum,vv);
    if(sNum[0]=='R'){
        r_instance.simulate(sNum);
    }
    
    //to make an instance of LCFS to simulate
    lcfs_sch l_instance(sNum,vv);
    if(sNum[0]=='L'){
        l_instance.simulate(sNum);
    }
    
    //to make an instance of SJF to simulate
    sjf_sch s_instance(sNum,vv);
    if(sNum[0]=='S'){
        s_instance.simulate(sNum);
    }
    
    //to print out the results
    for(int i=0; i<processSize;i++){
        printf("%04d: %4d %4d %4d %4d | %4d %4d %4d %4d\n", i, pArray[i].AT, pArray[i].TC, pArray[i].CB, pArray[i].IO, pArray[i].FT, pArray[i].TT, pArray[i].IT, pArray[i].CW);
    }
    cpuUtiliaztion = (double)100*CPUsum/timer;
    ioUtilization = (double)100*IOsum/timer;
    avgTT = (double)turnAroundSum/processSize;
    avgCW = (double)CPUWaitSum/processSize;
    proNum = (double)100*processSize/timer;
    
    //summary information
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",timer,cpuUtiliaztion,ioUtilization,avgTT,avgCW,proNum);
    
    return 0;
    
}


