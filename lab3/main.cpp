#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <list>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int physicalFrame = 32;//the size of physical frame
int clockH=0;//the hand in clock algorithm
int nruNum = 0;//the number of NRU
int pIndex;//the picked index

char pp = 'z';    //print 'p'
char fp = 'z';    //print 'f'
char Op = 'z';    //print 'O'
char Pp = NULL; //print 'P'
char Fp = NULL; //print 'F'
char Sp = NULL; //print 'S'
char ap = 'z';    //print 'a'


struct globalVar {
    char *algorithm;   //algorithms name, including based on physical frames and based on virtual pages(l,r,f,s,c,a,N,C,A)
    char *option;      //O,P,F,S option
    char *frameNum;    //frame number
    char *rfile;       //random file
    char *ifile;       //inputfile
} args;

struct pageTbaleEntry {
    unsigned   present : 1;   // 1 PRESENT bit
    unsigned   modified : 1;   // 1 MODIFIED bit
    unsigned   referenced : 1;   // 1 REFERENCED bit
    unsigned    pagedout : 1;// 1 PAGEDOUT bit
    unsigned    physicalFrameNum : 6;    //physical frame n
    unsigned    index : 10;
    
};

struct physicalFrames{
    unsigned virtualPageNum : 7; //virtual page n
    unsigned emptySpace : 1; // indicate if the physicalframes is emptySpace
    unsigned index : 6;
};

struct statss{
    unsigned long count;
    unsigned long unmap;
    unsigned long map;
    unsigned long pageins;
    unsigned long pageouts;
    unsigned long zero;
    unsigned long cost;
};

struct q{ //queue
    int l; //length
    struct physicalFrames *physicalframes[200];
};

struct ram{
    int n;
    int index;
    int ramdom[99999];
};

struct inst{
    int instNew;    //check if the instructuction is new
    int readWrite;    //read or write
    int virtualPage;    // virtual page n
    int index;
};


struct NRUSimulate{
    struct pageTbaleEntry *c0[100];
    struct pageTbaleEntry *c1[100];
    struct pageTbaleEntry *c2[100];
    struct pageTbaleEntry *c3[100];
    int c0Length;
    int c1Length;
    int c2Length;
    int c3Length;
};

struct ageing{
    unsigned age : 32;
    unsigned physicalFrameNum : 10;    //index of related pte
    unsigned virtualPageNum : 10;
    
};

// !!!variables moved from main
    FILE *rfile;
  
    int m= 0;//instruct index
    int opt = 0;
    
    struct ram myram;
    struct pageTbaleEntry pagetableentry[64];
    struct physicalFrames physicalframes[100];    // define a array that large enough is ok
    struct physicalFrames *physicalframesrep;
    struct q qq;
    struct ageing age[100];
    struct statss stats;
    struct NRUSimulate nru;
    FILE *input;   
    struct inst instruct;

struct random{
    void (*addQueue) (struct q *q, struct physicalFrames *physicalframes);
    void (*uptQ) (struct q *q);
    struct physicalFrames* (*pick) (struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry, struct ram *ram);
};

struct allStr{ //random,paging,fifo,secondChance,clockPF,clockVP
    void (*addQueue) (struct q *q, struct physicalFrames *physicalframes);
    void (*uptQ) (struct q *q);
    struct physicalFrames* (*pick) (struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry);};

struct LRU{
    struct allStr basicstruct;
    
    void (*transfer) (struct q *q, int physicalFrameNum);
};

struct NRU{
    void (*addQueue) (struct q *q, struct physicalFrames *physicalframes);
    void (*uptQ) (struct q *q);
    struct NRUSimulate (*mynewTable) (struct pageTbaleEntry *pagetableentry);
    void (*clear) (struct pageTbaleEntry *pagetableentry);
    struct physicalFrames* (*pick) (struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry, struct NRUSimulate *nru, struct ram *ram);
};

struct ageingVP{
    void (*addQueue) (struct q *q, struct physicalFrames *physicalframes);
    void (*uptQ) (struct q *q);
    void (*uptEmpt) (int emptyIndex, struct ageing *age);
    void (*uptExt) (int extIndex, struct ageing *age);
    void (*uptRept) (struct inst instruct, struct pageTbaleEntry *pagetableentry, struct ageing *age);
    struct physicalFrames* (*pick) (struct ageing *age,struct pageTbaleEntry *pagetableentry, struct physicalFrames *physicalframes);
};

struct ageingPF{
    void (*addQueue) (struct q *q, struct physicalFrames *physicalframes);
    void (*uptQ) (struct q *q);
    void (*uptEmpt) (int emptyIndex, struct ageing *age);
    void (*uptExt) (int extIndex, struct ageing *age);
    void (*uptRept)(int physicalFrame, int pickedIndex, struct pageTbaleEntry *pagetableentry, struct ageing *age, struct physicalFrames *physicalframes);
    struct physicalFrames* (*pick) (int physicalFrame, struct ageing *age,struct pageTbaleEntry *pagetableentry, struct physicalFrames *physicalframes);
};

//Method

struct ram getRandom(FILE *input){
    
    struct ram myram;
    
    fscanf(input," %d ",&myram.n);
    if(myram.n>99999){
        printf("ramdon nber exit the maximun of array!\n");
        exit(0);
    }
    
    if((input)==NULL){
        printf("ERROR: can not open random nber rfile\n");
        exit(0);
    }
    
    for(int j=0;j<myram.n;j++){
        fscanf(input," %d ",&myram.ramdom[j]);
    }
    return myram;
}

int myrandom(int d,struct ram *ram) {
    if(ram->index > ram->n-1){
        ram->index = 0;
    }
    ram->index ++;
    return  (ram->ramdom[ram->index-1] % d);
}


struct inst getInst(FILE *input){
    struct inst instruct;
    instruct.index = 0;
    if((input)==NULL){
        printf("ERROR: can not open input file\n");
        //getchar();
        exit(0);
    }
    while(!feof(input)){
        instruct.instNew = 0;
        char c = fgetc(input);
        if(c == '#'){
            while(c!='\n'){c = fgetc(input);}
        }
        else if(c == '\n'){c = fgetc(input);}
            else{
                 if(c == '1'){
                    instruct.readWrite = 1;
                    instruct.instNew = 1;
                }
                 else if(c == '0'){
                    instruct.readWrite = 0;
                    instruct.instNew = 1;
                }
              
                fscanf(input," %d ",&instruct.virtualPage);
                instruct.index=instruct.index+1;
                if (instruct.instNew ==1)
                    printf("readWrite=%d, virtualPage=%d\n",instruct.readWrite,instruct.virtualPage);
                    
    }
    }
    return instruct;
}

void addQ(struct q *q, struct physicalFrames *physicalframes){ //add queue fifo,secondchance,lru
    //add physicalframes to tail
    q->physicalframes[q->l] = physicalframes;
    q->l++;
}

void uptQ(struct q *q){//update queue fifo, secondchance
    //after pick up, move the pick up to tail, every element move foreadWriteard
    addQ(q, q->physicalframes[0]);
    for(int i=0;i<q->l-1;i++){
        q->physicalframes[i] = q->physicalframes[i+1];
    }
    q->l--;
}

void addNullQ(struct q *q){}//add null random

void addOtherNullQ(struct q *q, struct physicalFrames *physicalframes){}//add null

//fifo

struct physicalFrames* fifoPick(struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry){
    
    return q->physicalframes[0];
}

//second chance
struct physicalFrames* secondPick(struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry){
    while(pagetableentry[q->physicalframes[0]->virtualPageNum].referenced == 1){
        pagetableentry[q->physicalframes[0]->virtualPageNum].referenced = 0;
        addNullQ(q);
    }
    return q->physicalframes[0];
    
}
//random
struct physicalFrames* ramPick(struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry, struct ram *ram){
    int index = myrandom(physicalFrame,ram);
    return &physicalframes[index];
}

//lru
void lruTrsf(struct q *q, int physicalFrameNum){ // index is physicalframesn
    //move physicalframes[qIndex] to tail, then move foreadWriteard
    int j=0;
    while(q->physicalframes[j]->index != physicalFrameNum){
        j=j+1;    //calculate the index of physicalframesn in q
    }
    q->physicalframes[q->l] = q->physicalframes[j];
    for(int i=j;i<q->l;i++){
        q->physicalframes[i] = q->physicalframes[i+1];
    }
}

struct physicalFrames* lrupick(struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry){
    return q->physicalframes[0];
}

//clock-c

struct physicalFrames* clockPFPick(struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry){
    int i;
    
    while(pagetableentry[q->physicalframes[clockH]->virtualPageNum].referenced == 1){
        pagetableentry[q->physicalframes[clockH]->virtualPageNum].referenced = 0;
        clockH=clockH+1;
        if(clockH == physicalFrame){clockH=0;}
    }
    i = clockH;
    clockH=clockH+1;
    if(clockH == physicalFrame) {clockH = 0;}
    return q->physicalframes[i];
    
}

//clock-C
struct physicalFrames* clockVPPick(struct q *q, struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry){
    int i;
    struct physicalFrames* physicalframes2;
    while(pagetableentry[clockH].present ==0){
         if(clockH == 64) {clockH = 0;}
         clockH=clockH+1;
       
    }
    while(pagetableentry[clockH].referenced == 1){
        pagetableentry[clockH].referenced = 0;
            if(clockH == 64){clockH = 0;}
        clockH=clockH+1;
    
        while(pagetableentry[clockH].present ==0){
            if(clockH == 64) {clockH = 0;}
            clockH = clockH+1;
            
        }
    }
    i = clockH;
    clockH= clockH+1;
    if(clockH == 64) {clockH = 0;}
    physicalframes2 = &physicalframes[pagetableentry[i].physicalFrameNum];
    return physicalframes2;
    
}
//NRU

struct NRUSimulate nrumynewTable(struct pageTbaleEntry *pagetableentry){
    struct NRUSimulate nrusmlt;
    nrusmlt.c0Length = 0;
    nrusmlt.c1Length = 0;
    nrusmlt.c2Length = 0;
    nrusmlt.c3Length = 0;
    
    for(int i=0;i<64;i++){
        
        if(pagetableentry[i].present == 1){
            if((pagetableentry[i].modified == 0) && (pagetableentry[i].referenced == 0)){
               nrusmlt.c0[nrusmlt.c0Length] = &pagetableentry[i];
                nrusmlt.c0Length=nrusmlt.c0Length+1;
            }
            else if((pagetableentry[i].modified == 1) && (pagetableentry[i].referenced == 0)){
                nrusmlt.c1[nrusmlt.c1Length] = &pagetableentry[i];
                nrusmlt.c1Length=nrusmlt.c1Length+1;
            }
            else if((pagetableentry[i].modified == 0) && (pagetableentry[i].referenced == 1) ){
                nrusmlt.c2[nrusmlt.c2Length] = &pagetableentry[i];
                nrusmlt.c2Length=nrusmlt.c2Length+1;
            }
            else if((pagetableentry[i].modified == 1) && (pagetableentry[i].referenced == 1)){
                nrusmlt.c3[nrusmlt.c3Length] = &pagetableentry[i];
                nrusmlt.c3Length=nrusmlt.c3Length+1;
            }
        }
    }
    
    return nrusmlt;
}

void nruclear(struct pageTbaleEntry *pagetableentry){ // every 10 pages, clear R bit
    for(int i=0;i<64;i++){
        if((pagetableentry[i].referenced == 1) && (pagetableentry[i].present == 1) ){
            pagetableentry[i].referenced = 0;
        }
    }
    if(ap == 'a')
        printf(" @@ reset NRU refbits while walking PTE\n");
}

struct physicalFrames* nrupick(struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry, struct NRUSimulate *nru, struct ram *ram){
    struct physicalFrames* physicalframes2;
    int i,j;
    
    if(nru->c0Length > 0){
        j= myrandom(nru->c0Length,ram);
        physicalframes2 = &physicalframes[nru->c0[j]->physicalFrameNum];
        if(ap == 'a'){
            printf(" @@ lowest_class=0: selidx=%d from",j);
            for(i=0;i<nru->c0Length;i++){
                printf(" %d",nru->c0[i]->index);
            }
            printf("\n");
        }
    }
    else if(nru->c1Length >0){
        j = myrandom(nru->c1Length,ram);
        physicalframes2 = &physicalframes[nru->c1[j]->physicalFrameNum];
        if(ap == 'a'){
            printf(" @@ lowest_class=1: selidx=%d from",j);
            for(i=0;i<nru->c1Length;i++){
                printf(" %d",nru->c1[i]->index);
            }
            printf("\n");
        }
    }
    else if(nru->c2Length >0){
        j = myrandom(nru->c2Length,ram);
        physicalframes2 = &physicalframes[nru->c2[j]->physicalFrameNum];
        if(ap == 'a'){
            printf(" @@ lowest_class=2: selidx=%d from",j);
            for(i=0;i<nru->c2Length;i++){
                printf(" %d",nru->c2[i]->index);
            }
            printf("\n");
        }
        
        
    }
    else if(nru->c3Length >0){
        j = myrandom(nru->c3Length,ram);
        physicalframes2 = &physicalframes[nru->c3[j]->physicalFrameNum];
        if(ap == 'a'){
            printf(" @@ lowest_class=3: selidx=%d from",j);
            for(i=0;i<nru->c3Length;i++){
                printf(" %d",nru->c3[i]->index);
            }
            printf("\n");
        }
        
    }
    else{
        printf("nru_class select error\n");
        exit(0);
    }
    
    return physicalframes2;
    
}

//ageingVP

void updateageAemp(int emptyIndex, struct ageing *age){
    
    age[emptyIndex].age = age[emptyIndex].age | (1 << 31);
}

void updateageAexist(int extIndex, struct ageing *age){
    
    age[extIndex].age = age[extIndex].age | (1 << 31);
}



void updateageArep(struct inst instruct, struct pageTbaleEntry *pagetableentry, struct ageing *age){
    for(int i=0;i<64;i++){
        if(pagetableentry[i].present == 1){
            age[i].age >>=1;
        }
    }
    age[instruct.virtualPage].age = 0;
    age[instruct.virtualPage].age = age[instruct.virtualPage].age | (1 << 31);
    
}

struct physicalFrames* ageApick(struct ageing *age,struct pageTbaleEntry *pagetableentry, struct physicalFrames *physicalframes){
    unsigned int min = 1234567890;
    unsigned int pickedIndex;
    
    for(int i=0;i<64;i++){
        if(pagetableentry[i].present == 1){
            if(age[i].age < min){
                min = age[i].age;
                pickedIndex = i;
            }
        }
    }
    for(int j=0;j<64;j++){
        pagetableentry[j].referenced = 0;
    }
    return &physicalframes[pagetableentry[pickedIndex].physicalFrameNum];
}


//ageingPF

void updateageaemp(int emptyIndex, struct ageing *age){
    
    age[emptyIndex].age = age[emptyIndex].age | (1 << 31);
    
}

void updateageaexist(int extIndex, struct ageing *age){
    
    age[extIndex].age = age[extIndex].age | (1 << 31);
}

void updateagearep(int physicalFrame, int pickedIndex, struct pageTbaleEntry *pagetableentry, struct ageing *age, struct physicalFrames *physicalframes){
    for(int i=0;i<physicalFrame;i++){
        if(pagetableentry[physicalframes[i].virtualPageNum].present == 1){
            age[i].age >>=1;
        }
    }
    age[pickedIndex].age = 0;
    age[pickedIndex].age = age[pickedIndex].age | (1 << 31);
    
}

struct physicalFrames* ageapick(int physicalFrame, struct ageing *age,struct pageTbaleEntry *pagetableentry, struct physicalFrames *physicalframes){
    unsigned int min = 1234567890;
    unsigned int pickedIndex;
    
    for(int i=0;i<physicalFrame;i++){
        if(pagetableentry[physicalframes[i].virtualPageNum].present == 1){
            if(age[i].age < min){
                min = age[i].age;
                pickedIndex = i;
                pIndex = i;
            }
        }
    }
    for(int j=0;j<64;j++){
        pagetableentry[j].referenced = 0;
    }
    
    return &physicalframes[pickedIndex];
}

//printO
/*void printOinstruct(struct inst instruct){
 printf("==> inst: %d %d\n",instruct.readWrite,instruct.virtualPage);
 }
 */
/*
 void printOunmap(struct inst instruct, int virtualPageNum, int physicalFrameNum){
 printf("%d: UNMAP%4d%4d\n",instruct.index, virtualPageNum, physicalFrameNum);
 }
 */
/*
 void printOmap(struct inst instruct, int virtualPageNum, int physicalFrameNum){
 printf("%d: MAP%6d%4d\n",instruct.index,virtualPageNum, physicalFrameNum);
 }
 */
/*
 void printOin(struct inst instruct, int virtualPageNum, int physicalFrameNum){
 printf("%d: IN%7d%4d\n", instruct.index, virtualPageNum, physicalFrameNum);
 }
 */
/*
 void printOout(struct inst instruct , int virtualPageNum, int physicalFrameNum){
 printf("%d: OUT%6d%4d\n", instruct.index, virtualPageNum, physicalFrameNum);
 }
 */
/*
 void printOzero(struct inst instruct, int physicalFrameNum){
 printf("%d: ZERO%9d\n", instruct.index, physicalFrameNum);
 }
 */

//printP

void printP(int l, struct pageTbaleEntry *pagetableentry){
    for(int i=0;i<l;i++){
        if(pagetableentry[i].present == 1){
            printf("%d:",i);
            if(pagetableentry[i].referenced == 1) printf("R"); else printf("-");
            if(pagetableentry[i].modified == 1) printf("M"); else printf("-");
            if(pagetableentry[i].pagedout == 1) printf("S"); else printf("-");
            printf(" ");
        }
        else if(pagetableentry[i].present == 0){
            if(pagetableentry[i].pagedout == 0) printf("* ");
            if(pagetableentry[i].pagedout == 1) printf("# ");
        }
    }
    printf("\n");
}


//print F, random
void prt1(int physicalFrame, struct physicalFrames *physicalframes, struct q *qq){
    for(int i=0;i<physicalFrame;i++){
        if(physicalframes[i].emptySpace == 0) printf("* ");
        else printf("%d ",physicalframes[i].virtualPageNum);
    }
    
    printf("\n");
}

//print fifo,second chance, lru
/*void prt2(int physicalFrame, struct physicalFrames *physicalframes, struct q *qq){
    for(int i=0;i<physicalFrame;i++){
        if(physicalframes[i].emptySpace == 0) printf("* ");
        else printf("%d ",physicalframes[i].virtualPageNum);
    }
    printf(" || ");
    for(int j=0; j<qq->l;j++){
        printf("%d ",qq->physicalframes[j]->index);
    }
    printf("\n");
}
*/

//clock print
void cloprintf(int physicalFrame, struct physicalFrames *physicalframes, struct q *qq){
    for(int i=0;i<physicalFrame;i++){
        if(physicalframes[i].emptySpace == 0) printf("* ");
        else printf("%d ",physicalframes[i].virtualPageNum);
    }
    printf(" || ");
    printf("hand = %d",clockH);
    printf("\n");
}

//ageingVP print
void ageingVPprintf(int physicalFrame, int virtualPagel, struct pageTbaleEntry *pagetableentry, struct physicalFrames *physicalframes,struct ageing *age){
    for(int i=0;i<physicalFrame;i++){
        if(physicalframes[i].emptySpace == 0) printf("* ");
        else printf("%d ",physicalframes[i].virtualPageNum);
    }
    printf(" || ");
    for(int j=0;j<virtualPagel;j++){
        if(pagetableentry[j].present == 0)    printf("* ");
        if(pagetableentry[j].present == 1){
            printf("%d:%x ",j,age[j].age);
        }
    }
    
    printf("\n");
}

//ageingPF print
void ageingPFprintf(int physicalFrame, int virtualPagel, struct pageTbaleEntry *pagetableentry, struct physicalFrames *physicalframes,struct ageing *age){
    for(int i=0;i<physicalFrame;i++){
        if(physicalframes[i].emptySpace == 0) printf("* ");
        else printf("%d ",physicalframes[i].virtualPageNum);
    }
    printf(" || ");
    for(int j=0;j<physicalFrame;j++){
        if(pagetableentry[physicalframes[j].virtualPageNum].present == 0)    printf("* ");
        if(pagetableentry[physicalframes[j].virtualPageNum].present == 1){
            printf("%d:%x ",j,age[j].age);
        }
    }
    
    printf("\n");
}


class mmu{
    public:

    virtual void func1(){};
    virtual void func2(){};
    virtual void func3(){};
    virtual void func4(){};

    void sim(){ // !!!Copied from main
        while(!feof(input)){
        
            instruct.index = 0;
            instruct.instNew = 0;
            
            char c = fgetc(input);
            if(c == '#'){
                do{
                    c = fgetc(input);
                }while(c != '\n');
            }
            else if(c == '\n')
                c = fgetc(input);
            else{
                if(c == '0'){
                    instruct.readWrite = 0;
                    instruct.instNew = 1;
                }
                else if(c == '1'){
                    instruct.readWrite = 1;
                    instruct.instNew = 1;
                }
                fscanf(input," %d ",&instruct.virtualPage);
                instruct.index = m;
                m=m+1;
                
            }
            
            if(instruct.instNew==1){
                stats.count++; //cal n of instruct
                if(Op == 'O')
                    printf("==> inst: %d %d\n",instruct.readWrite,instruct.virtualPage);
                   // printOinstruct(instruct);
                
                //first check if page is already in pysical frame***/
                if(pagetableentry[instruct.virtualPage].present == 1){
                    
                    if(instruct.readWrite == 1){ // M only changed 0->1
                        pagetableentry[instruct.virtualPage].modified = instruct.readWrite;
                    }
                    pagetableentry[instruct.virtualPage].referenced = 1;
                    
                    // !!!replace by virtual function 1
                    func1();
                    /*
                    if(strcmp(args.algorithm,"l")==0){
                        lruTrsf(&qq,pagetableentry[instruct.virtualPage].physicalFrameNum);
                    }
                    
                    else if (strcmp(args.algorithm,"a")==0){
                        updateageaexist(pagetableentry[instruct.virtualPage].physicalFrameNum,age);
                    }
                    
                    else if(strcmp(args.algorithm,"A")==0){
                        updateageAexist(instruct.virtualPage,age);
                    }
                    */
                }
                
                else{
                    int replace = 1;
                    //if not in physicalframes, check if there are emptySpace physicalframes to save content
                    for(int i=0;i<physicalFrame;i++){
                        if(physicalframes[i].emptySpace==0){
                           
                            pagetableentry[instruct.virtualPage].present = 1;
                            pagetableentry[instruct.virtualPage].modified = instruct.readWrite;
                            pagetableentry[instruct.virtualPage].referenced = 1;
                            physicalframes[i].index = i;
                            pagetableentry[instruct.virtualPage].physicalFrameNum = i;
                            
                            physicalframes[i].virtualPageNum = instruct.virtualPage;
                            
                            // !!!replace by virtual function 2
                            func2();
                            /*                            
                            pagep->addQueue(&qq,&physicalframes[i]);
                            if(strcmp(args.algorithm,"a")==0){
                                updateageaemp(i,&age[0]);
                            }
                            else if(strcmp(args.algorithm,"A")==0){
                                updateageAemp(physicalframes[i].virtualPageNum,age);
                            }
                            */
                           
                            replace = 0;
                            
                            if(Op == 'O'){
                                //printOzero(instruct, physicalframes[i].index);
                                 printf("%d: ZERO%9d\n", instruct.index, physicalframes[i].index);
                                
                                printf("%d: MAP%6d%4d\n",instruct.index,instruct.virtualPage, physicalframes[i].index);
                                //printOmap(instruct, instruct.virtualPage, physicalframes[i].index);
                            }
                            physicalframes[i].emptySpace = 1;
                        
                            stats.map++;
                            stats.zero++;
                            break;
                        }
                    }
                    
                    //if no emptySpace physicalframes, start page replacement algorithm
                    if(replace == 1){
                        // !!!replace by virtual function 3
                        func3();
                        /*
                        if(strcmp(args.algorithm,"N")==0){
                            nru = nrumynewTable(pagetableentry);
                            physicalframesrep = nrupick(physicalframes,pagetableentry,&nru,&myram);
                            nruNum=nruNum+1;
                            if(nruNum % 10 ==0){
                                nruclear(pagetableentry);
                            }
                        }
                        
                        else if(strcmp(args.algorithm,"r")==0)
                            physicalframesrep = randp->pick(&qq,physicalframes,pagetableentry,&myram);
                        
                        else if(strcmp(args.algorithm,"a")==0){
                            physicalframesrep = ageapick(physicalFrame,age,pagetableentry,physicalframes);
                            updateagearep(physicalFrame,pIndex,pagetableentry,age,physicalframes);
                        }
                        
                        else if(strcmp(args.algorithm,"A")==0){
                            physicalframesrep = ageApick(age,pagetableentry,physicalframes);
                            updateageArep(instruct,pagetableentry,age);
                        }
                     
                        else{
                            physicalframesrep = pagep->pick(&qq,physicalframes,pagetableentry);
                            
                        }
                        */
                        
                        int virtualPageNumrep, physicalFrameNumrep;
                        
                        virtualPageNumrep = physicalframesrep->virtualPageNum;
                        physicalFrameNumrep = physicalframesrep->index;
                        
                        pagetableentry[physicalframesrep->virtualPageNum].present = 0; // UNMAP and become invalid
                        stats.unmap=stats.unmap+1;
                        
                        if(Op == 'O')
                        printf("%d: UNMAP%4d%4d\n",instruct.index, virtualPageNumrep, physicalFrameNumrep);
                        //printOunmap(instruct,virtualPageNumrep, physicalFrameNumrep);
                        
                        if(pagetableentry[physicalframesrep->virtualPageNum].modified == 1){
                            pagetableentry[physicalframesrep->virtualPageNum].pagedout = 1; //SWAP OUT
                            if(Op == 'O')
                            printf("%d: OUT%6d%4d\n", instruct.index, virtualPageNumrep, physicalFrameNumrep);
                            //printOout(instruct,virtualPageNumrep, physicalFrameNumrep);
                            pagetableentry[physicalframesrep->virtualPageNum].modified =0; //after pagedout,M 1->0
                            stats.pageouts=stats.pageouts+1;
                            
                        }
                        if(pagetableentry[instruct.virtualPage].pagedout == 0){
                            // ZERO
                            if(Op == 'O')
                            printf("%d: ZERO%9d\n", instruct.index, physicalFrameNumrep);
                            //printOzero(instruct, physicalFrameNumrep);
                            stats.zero=stats.zero+1;
                            
                        }else{
                            //PAGE IN
                            if(Op == 'O')
                                printf("%d: IN%7d%4d\n", instruct.index, instruct.virtualPage, physicalFrameNumrep);
                                //printOin(instruct,instruct.virtualPage, physicalFrameNumrep);
                            stats.pageins=stats.pageins+1;
                        }
                        
                        //MAP
                        physicalframesrep->virtualPageNum = instruct.virtualPage;
                        pagetableentry[instruct.virtualPage].present = 1;
                        pagetableentry[instruct.virtualPage].modified = instruct.readWrite;
                        pagetableentry[instruct.virtualPage].referenced = 1;
                        pagetableentry[instruct.virtualPage].physicalFrameNum = physicalframesrep->index;
                        stats.map=stats.map+1;
                        
                        
                        if(Op == 'O')
                           printf("%d: MAP%6d%4d\n",instruct.index,instruct.virtualPage,physicalFrameNumrep);
                        //printOmap(instruct,instruct.virtualPage,physicalFrameNumrep);
                        
                        // !!!replace by virtual function 4
                        func4();
                        /*
                        pagep->uptQ(&qq);
                        */
                    }
                }
                if(pp == 'p'){printP(64,pagetableentry);}
                
                if(fp == 'f'){cloprintf(physicalFrame,physicalframes,&qq);}
                
            }// end of new each instructuction
            
        }//end all  instruct
    
    
        stats.cost = stats.map*400 + stats.unmap * 400 + stats.pageins*3000 + stats.pageouts*3000 + stats.zero*150 + stats.count;
        
        if(Pp == 'P'){
            printP(64,&pagetableentry[0]);
        }
        if(Fp == 'F'){
            prt1(physicalFrame,physicalframes,&qq);
        }
        if(Sp == 'S'){
            printf("SUM %d U=%d M=%d I=%d O=%d Z=%d ===> %llu\n", stats.count, stats.unmap, stats.map, stats.pageins, stats.pageouts, stats.zero, stats.cost);
        }
    }
};

//!!!functions copied from above
class ran_mmu : public mmu{

    void addOtherNullQ(struct q *q, struct physicalFrames *physicalframes){}
    void addNullQ(struct q *q){}//add null random
    struct physicalFrames* ramPick(struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry, struct ram *ram){
        int index = myrandom(physicalFrame,ram);
        return &physicalframes[index];
    }

    void func1(){};

    void func2(){
        addOtherNullQ(&qq,physicalframes);
    };

    void func3(){
        physicalframesrep = ramPick(&qq,physicalframes,pagetableentry,&myram);
    }

    void func4(){
        addNullQ(&qq);
    }
};

class fifo_mmu : public mmu{
    void addOtherNullQ(struct q *q, struct physicalFrames *physicalframes){}
    void addNullQ(struct q *q){}//add null random
    struct physicalFrames* ramPick(struct q *q,  struct physicalFrames *physicalframes, struct pageTbaleEntry *pagetableentry, struct ram *ram){
        int index = myrandom(physicalFrame,ram);
        return &physicalframes[index];
    }
    
    void func1(){};
    
    void func2(){
        addNullQ(&qq);
    };
    
    void func3(){
        physicalframesrep = pick(&qq,physicalframes,pagetableentry);
    }
    
    void func4(){
        addNullQ(&qq);
    }

};
//start main()

int main(int argc, char *argv[]){
    
    qq.l = 0;
    // Initialize args before we get to work.
    args.algorithm = "l";
    args.option = "";
    args.frameNum = NULL;
    
    opt = getopt( argc, argv, "a:o:f:");
    while( opt != -1 ) {
        switch( opt ) {
            case 'a':
                args.algorithm = optarg;
                break;
                
            case 'f':
                args.frameNum = optarg;
                physicalFrame = atoi(args.frameNum);
                break;
                
            case 'o':
                args.option = optarg;
                break;
                
        }
        opt = getopt( argc, argv, "a:o:f:");
    }
    
    int index = optind;
    args.ifile = argv[index];
    index=index+1;
    args.rfile = argv[index];
    
    //multi
    struct random rand, *randp;
    //struct allStr rand,*randp;
    struct allStr page, *pagep, fifo, *fifop, sec,*secp, clc, *clcp, clC,*clCp;
    struct LRU lru,*lrup;
    struct NRU nru2,*nru2p;
    struct ageingPF agea,*ageap;
    struct ageingVP ageA,*ageAp;
    
    pagep = &page;
    fifop = &fifo;
    secp = &sec;
    lrup = &lru;
    clcp = &clc;
    clCp = &clC;
    nru2p = &nru2;
    ageap = &agea;
    ageAp = &ageA;
    randp = &rand;
    
    //!!!
    /*
    rand.addQueue = addOtherNullQ;
    rand.uptQ = addNullQ;
    rand.pick = ramPick;
    */
    ran_mmu r;
    
    fifo.addQueue = addQ;
    fifo.uptQ = uptQ;
    fifo.pick = fifoPick;
    
    sec.addQueue = addQ;
    sec.uptQ = addNullQ;
    sec.pick = secondPick;
    
    lru.basicstruct.addQueue = addQ;
    lru.basicstruct.uptQ = uptQ;
    lru.basicstruct.pick = lrupick;
    lru.transfer = lruTrsf;
    
    clc.addQueue = addQ;
    clc.uptQ = addNullQ;
    clc.pick = clockPFPick;
    
    clc.addQueue = addQ;
    clc.uptQ = addNullQ;
    clc.pick = clockPFPick;
    
    nru2.addQueue = addOtherNullQ;
    nru2.uptQ = addNullQ;
    nru2.mynewTable = nrumynewTable;
    nru2.clear = nruclear;
    nru2.pick = nrupick;
    
    agea.addQueue = addOtherNullQ;
    agea.pick = ageapick;
    agea.uptEmpt = updateageaemp;
    agea.uptExt = updateageaexist;
    agea.uptQ = addNullQ;
    agea.uptRept = updateagearep;
    
    ageA.addQueue = addOtherNullQ;
    ageA.pick = ageApick;
    ageA.uptEmpt = updateageAemp;
    ageA.uptExt = updateageAexist;
    ageA.uptQ = addNullQ;
    ageA.uptRept = updateageArep;
    
    if(strcmp(args.algorithm,"N")==0){
        pagep = (struct allStr*)nru2p;
    }
    else if(strcmp(args.algorithm,"C")==0){
        pagep = (struct allStr*)clCp;
    }
    else if(strcmp(args.algorithm,"A")==0){
        pagep = (struct allStr*)ageAp;
    }
    else if(strcmp(args.algorithm,"l")==0){
        pagep = (struct allStr*)lrup;
    }
    else if(strcmp(args.algorithm,"r")==0){
        pagep = (struct allStr*)randp;
    }
    else if(strcmp(args.algorithm,"f")==0){
        pagep = (struct allStr*)fifop;
    }
    else if(strcmp(args.algorithm,"s")==0){
        pagep = (struct allStr*)secp;
    }
    else if(strcmp(args.algorithm,"c")==0){
        pagep = (struct allStr*)clcp;
    }
    
    else if(strcmp(args.algorithm,"a")==0){
        pagep = (struct allStr*)ageap;
    }
    else {
        pagep = (struct allStr*)lrup;
    }
    
    //matching the -oOphysicalFramesS option
    if(strcmp(args.option,"")==0){
        printf("please type in the -o[OphysicalFramesS] option to specify output format\n");
        exit(0);
    }
    if(strstr(args.option,"O")!=NULL){
        Op = 'O';
    }
    if(strstr(args.option,"P")!=NULL){
        Pp = 'P';
    }
    if(strstr(args.option,"S")!=NULL){
        Sp = 'S';
    }
    if(strstr(args.option,"F")!=NULL){
        Fp = 'F';
    }
    
    rfile = fopen(args.rfile,"r");
    myram = getRandom(rfile);
    myram.index = 0;
    
    input = fopen(args.ifile,"r");
    
    //initilize
    stats.pageins = 0;
    stats.count = 0;
    stats.map = 0;
    stats.pageouts = 0;
    stats.cost = 0;
    stats.unmap = 0;
    stats.zero = 0;
    
    for(int i=0;i<64;i++){
        pagetableentry[i].index = i;
        pagetableentry[i].present = 0;
        pagetableentry[i].modified = 0;
        pagetableentry[i].referenced = 0;
        pagetableentry[i].pagedout = 0;
        age[i].age = 0;
        age[i].virtualPageNum = i;
        
    }
    
    for(int i=0;i<physicalFrame;i++){
        physicalframes[i].emptySpace = 0;
        age[i].physicalFrameNum = i;
    }
    
    //start loading instructuntion
    if((input)==NULL){
        printf("ERROR: can not open input file\n");
        //getchar();
        exit(0);
    }
    //!!!Later use switch-case to choose among different algorithms
    r.sim();
    /*
    while(!feof(input)){
        
        instruct.index = 0;
        instruct.instNew = 0;
        
        char c = fgetc(input);
        if(c == '#'){
            do{
                c = fgetc(input);
            }while(c != '\n');
        }
        else if(c == '\n')
            c = fgetc(input);
        else{
            if(c == '0'){
                instruct.readWrite = 0;
                instruct.instNew = 1;
            }
            else if(c == '1'){
                instruct.readWrite = 1;
                instruct.instNew = 1;
            }
            fscanf(input," %d ",&instruct.virtualPage);
            instruct.index = m;
            m=m+1;
            
        }
        
        if(instruct.instNew==1){
            stats.count++; //cal n of instruct
            if(Op == 'O')
                printf("==> inst: %d %d\n",instruct.readWrite,instruct.virtualPage);
               // printOinstruct(instruct);
            
            //first check if page is already in pysical frame
            if(pagetableentry[instruct.virtualPage].present == 1){
                
                if(instruct.readWrite == 1){ // M only changed 0->1
                    pagetableentry[instruct.virtualPage].modified = instruct.readWrite;
                }
                pagetableentry[instruct.virtualPage].referenced = 1;
                
                if(strcmp(args.algorithm,"l")==0){
                    lruTrsf(&qq,pagetableentry[instruct.virtualPage].physicalFrameNum);
                }
                
                else if (strcmp(args.algorithm,"a")==0){
                    updateageaexist(pagetableentry[instruct.virtualPage].physicalFrameNum,age);
                }
                
                else if(strcmp(args.algorithm,"A")==0){
                    updateageAexist(instruct.virtualPage,age);
                }
                
            }
            
            else{
                int replace = 1;
                //if not in physicalframes, check if there are emptySpace physicalframes to save content
                for(int i=0;i<physicalFrame;i++){
                    if(physicalframes[i].emptySpace==0){
                       
                        pagetableentry[instruct.virtualPage].present = 1;
                        pagetableentry[instruct.virtualPage].modified = instruct.readWrite;
                        pagetableentry[instruct.virtualPage].referenced = 1;
                        physicalframes[i].index = i;
                        pagetableentry[instruct.virtualPage].physicalFrameNum = i;
                        
                        physicalframes[i].virtualPageNum = instruct.virtualPage;
                        
                        pagep->addQueue(&qq,&physicalframes[i]);
                        
                        if(strcmp(args.algorithm,"a")==0){
                            updateageaemp(i,&age[0]);
                        }
                        else if(strcmp(args.algorithm,"A")==0){
                            updateageAemp(physicalframes[i].virtualPageNum,age);
                        }
                       
                        replace = 0;
                        
                        if(Op == 'O'){
                            //printOzero(instruct, physicalframes[i].index);
                             printf("%d: ZERO%9d\n", instruct.index, physicalframes[i].index);
                            
                            printf("%d: MAP%6d%4d\n",instruct.index,instruct.virtualPage, physicalframes[i].index);
                            //printOmap(instruct, instruct.virtualPage, physicalframes[i].index);
                        }
                        physicalframes[i].emptySpace = 1;
                    
                        stats.map++;
                        stats.zero++;
                        break;
                    }
                }
                
                //if no emptySpace physicalframes, statrt page replacement algorithm
                if(replace == 1){
                    
                    if(strcmp(args.algorithm,"N")==0){
                        nru = nrumynewTable(pagetableentry);
                        physicalframesrep = nrupick(physicalframes,pagetableentry,&nru,&myram);
                        nruNum=nruNum+1;
                        if(nruNum % 10 ==0){
                            nruclear(pagetableentry);
                        }
                    }
                    
                    else if(strcmp(args.algorithm,"r")==0)
                        physicalframesrep = randp->pick(&qq,physicalframes,pagetableentry,&myram);
                    
                    else if(strcmp(args.algorithm,"a")==0){
                        physicalframesrep = ageapick(physicalFrame,age,pagetableentry,physicalframes);
                        updateagearep(physicalFrame,pIndex,pagetableentry,age,physicalframes);
                    }
                    
                    else if(strcmp(args.algorithm,"A")==0){
                        physicalframesrep = ageApick(age,pagetableentry,physicalframes);
                        updateageArep(instruct,pagetableentry,age);
                    }
                 
                    else{
                        physicalframesrep = pagep->pick(&qq,physicalframes,pagetableentry);
                        
                    }
                    
                    int virtualPageNumrep, physicalFrameNumrep;
                    
                    virtualPageNumrep = physicalframesrep->virtualPageNum;
                    physicalFrameNumrep = physicalframesrep->index;
                    
                    pagetableentry[physicalframesrep->virtualPageNum].present = 0; // UNMAP and become invalid
                    stats.unmap=stats.unmap+1;
                    
                    if(Op == 'O')
                    printf("%d: UNMAP%4d%4d\n",instruct.index, virtualPageNumrep, physicalFrameNumrep);
                    //printOunmap(instruct,virtualPageNumrep, physicalFrameNumrep);
                    
                    if(pagetableentry[physicalframesrep->virtualPageNum].modified == 1){
                        pagetableentry[physicalframesrep->virtualPageNum].pagedout = 1; //SWAP OUT
                        if(Op == 'O')
                        printf("%d: OUT%6d%4d\n", instruct.index, virtualPageNumrep, physicalFrameNumrep);
                        //printOout(instruct,virtualPageNumrep, physicalFrameNumrep);
                        pagetableentry[physicalframesrep->virtualPageNum].modified =0; //after pagedout,M 1->0
                        stats.pageouts=stats.pageouts+1;
                        
                    }
                    if(pagetableentry[instruct.virtualPage].pagedout == 0){
                        // ZERO
                        if(Op == 'O')
                        printf("%d: ZERO%9d\n", instruct.index, physicalFrameNumrep);
                        //printOzero(instruct, physicalFrameNumrep);
                        stats.zero=stats.zero+1;
                        
                    }else{
                        //PAGE IN
                        if(Op == 'O')
                            printf("%d: IN%7d%4d\n", instruct.index, instruct.virtualPage, physicalFrameNumrep);
                            //printOin(instruct,instruct.virtualPage, physicalFrameNumrep);
                        stats.pageins=stats.pageins+1;
                    }
                    
                    //MAP
                    physicalframesrep->virtualPageNum = instruct.virtualPage;
                    pagetableentry[instruct.virtualPage].present = 1;
                    pagetableentry[instruct.virtualPage].modified = instruct.readWrite;
                    pagetableentry[instruct.virtualPage].referenced = 1;
                    pagetableentry[instruct.virtualPage].physicalFrameNum = physicalframesrep->index;
                    stats.map=stats.map+1;
                    
                    
                    if(Op == 'O')
                       printf("%d: MAP%6d%4d\n",instruct.index,instruct.virtualPage,physicalFrameNumrep);
                    //printOmap(instruct,instruct.virtualPage,physicalFrameNumrep);
                    
                    pagep->uptQ(&qq);
                    
                }
            }
            if(pp == 'p'){printP(64,pagetableentry);}
            
            if(fp == 'f'){cloprintf(physicalFrame,physicalframes,&qq);}
            
        }// end of new each instructuction
        
    }//end all  instruct
    
    
    stats.cost = stats.map*400 + stats.unmap * 400 + stats.pageins*3000 + stats.pageouts*3000 + stats.zero*150 + stats.count;
    
    if(Pp == 'P'){
        printP(64,&pagetableentry[0]);
    }
    if(Fp == 'F'){
        prt1(physicalFrame,physicalframes,&qq);
    }
    if(Sp == 'S'){
        printf("SUM %d U=%d M=%d I=%d O=%d Z=%d ===> %llu\n", stats.count, stats.unmap, stats.map, stats.pageins, stats.pageouts, stats.zero, stats.cost);
    }
    */
    return 0;
}