
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <math.h>
#include <string>

using namespace std;

string algorithm = " ";

int gt = 0;

int total_time = 0;
int tot_movement = 0;
double avg_turnaround = 0;
double avg_waittime = 0;
int max_waittime = 0;

int tb = 0;
int il = 0;
int fi = 0;
int fscanQ;

struct basic{
    int inq;
    int st;
    int et;
	int at;
	int wt;
    int tat;
    int track;
    int index;
    int pl;
    
};
struct h{
    int dir;
    int ctr;
    int etr;
};
struct q{
    struct basic *bi[5000];
    int l;
};

struct scdl{ //scheduler:fifo,sstf,scan,cscan,fscan
    void (*sq)(struct q *que, struct h *head);
};

int mh(struct h *hh){
    if(hh->etr < hh->ctr){
        hh->ctr -- ;
        return 1;
    }
    else if(hh->ctr < hh->etr){
        hh->ctr ++;
        return 1;
    }
    else{
        return 0;
    }
}

void rq(struct q *qq){
	int a;
	for(a=0;a<(qq->l)-1;a++){
		qq->bi[a] = qq->bi[a+1];
	}
	qq->l--;
}

void faq(struct q *qq, struct basic *bibi){
	qq->bi[qq->l] = bibi;
	qq->l++;
}


void fsq(struct q *que, struct h *head){}

int cmp(const void *p, const void *q){
    struct basic **pp = (basic**)p;
    struct basic **qq = (basic**)q;
    if((*pp)->pl - (*qq)->pl == 0){
        return (*pp)->index - (*qq)->index ;
    }
    else
        return ((*pp)->pl - (*qq)->pl);
}

// SSTF
void sstfSq(struct q *qq, struct h *hh){
    
    qsort(&(qq->bi[0]),qq->l,sizeof(qq->bi[0]),cmp);
    if(qq->bi[0]->track == hh->ctr){
        tb = 1;
    }
}

// SCAN
void scanSq(struct q *qq, struct h *hh){
    int mpl = 1000000;
    int ni = 100000;
    int tmp = 0;
    if(hh->dir == 1){
        
        for(int i=0;i<qq->l;i++){
            if(qq->bi[i]->track >= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                    if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                }
                else ni = i;
                mpl =  abs(qq->bi[i]->track - hh->ctr);
                tmp = 1;
                hh->dir = 1;
                if(qq->bi[ni]->track == hh->ctr){tb = 1;}
            }
        }
        
        if(tmp == 0){
            for(int i=0;i<qq->l;i++){
                if(qq->bi[i]->track <= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                    if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                        if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                    }
                    else ni = i;
                    mpl =  abs(qq->bi[i]->track - hh->ctr);
                    hh->dir = 0;
                    if(qq->bi[ni]->track == hh->ctr){tb = 1;}
                }
                
            }
        }
        
    }else{
        for(int i=0;i<qq->l;i++){
            if(qq->bi[i]->track <= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                    if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                }
                else ni = i;
                mpl =  abs(qq->bi[i]->track - hh->ctr);
                hh->dir = 0;
                tmp = 1;
                if(qq->bi[ni]->track == hh->ctr){tb = 1;}
            }
        }
        
        if(tmp == 0){
            for(int i=0;i<qq->l;i++){
                if(qq->bi[i]->track >= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                    if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                        if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                    }
                    else ni = i;
                    mpl =  abs(qq->bi[i]->track - hh->ctr);
                    hh->dir = 1;
                    if(qq->bi[ni]->track == hh->ctr){tb = 1;}
                }
            }
        }
    }
    struct basic *temp = qq->bi[ni];
    qq->bi[ni] = qq->bi[0];
    qq->bi[0] = temp;
    
}

// CSCAN
void cscanSq(struct q *qq, struct h *hh){
    int ni = 1000000;
    int mpl = 100000;
    int tmp = 0;
    
    for(int i=0;i<qq->l;i++){
        if(qq->bi[i]->track >= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
            if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
            }
            else ni = i;
            mpl =  abs(qq->bi[i]->track - hh->ctr);
            tmp = 1;
            hh->dir = 1;
            if(qq->bi[ni]->track == hh->ctr){tb = 1;}
        }
    }
    if(tmp == 0){
        for(int i=0;i<qq->l;i++){
            if(qq->bi[i]->track <= mpl){
                mpl =  qq->bi[i]->track;
                ni = i;
            }
        }
    }
    struct basic *temp = qq->bi[ni];
    qq->bi[ni] = qq->bi[0];
    qq->bi[0] = temp;
}

// FSCAN
void fscanSq(struct q *qq, struct h *hh){
   int ni = 1000000;
    int mpl = 10000;
    int tmp = 0;
    if(hh->dir == 1){
                    for(int i=0;i<qq->l;i++){
            if(qq->bi[i]->track >= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                    if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                }
                else ni = i;
                mpl =  abs(qq->bi[i]->track - hh->ctr);
                tmp = 1;
                hh->dir = 1;
                if(qq->bi[ni]->track == hh->ctr){tb = 1;}
            }
        }
        if(tmp == 0){
            for(int i=0;i<qq->l;i++){
                if(qq->bi[i]->track <= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                    if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                        if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                    }
                    else ni = i;
                    mpl =  abs(qq->bi[i]->track - hh->ctr);
                    hh->dir = 0;
                    if(qq->bi[ni]->track == hh->ctr){
                        tb = 1;
                    }
                }
                
            }
        }
        
    }else{
        for(int i=0;i<qq->l;i++){
            if(qq->bi[i]->track <= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                    if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                }
                else ni = i;
                mpl =  abs(qq->bi[i]->track - hh->ctr);
                hh->dir = 0;
                tmp = 1;
                if(qq->bi[ni]->track == hh->ctr){tb = 1;}
            }
        }
        if(tmp == 0){
            for(int i=0;i<qq->l;i++){
                if(qq->bi[i]->track >= hh->ctr && abs(qq->bi[i]->track - hh->ctr) <= mpl){
                    if(abs(qq->bi[i]->track - hh->ctr) == mpl){
                        if(qq->bi[i]->index < qq->bi[ni]->index){ni = i;}
                    }
                    else ni = i;
                    mpl =  abs(qq->bi[i]->track - hh->ctr);
                    hh->dir = 1;
                    if(qq->bi[ni]->track == hh->ctr){tb = 1;}
                }
            }
        }
    }
    struct basic *temp = qq->bi[ni];
    qq->bi[ni] = qq->bi[0];
    qq->bi[0] = temp;
}

int main(int argc, char *argv[]){
 
  int opt = 0;
    opt = getopt( argc, argv, "s:");
	while( opt != -1 ) {
		switch( opt ) {
			case 's':
				algorithm = optarg;
				break;
		}
		opt = getopt( argc, argv, "s:");
	}
  
	char *filepath = argv[optind];
    FILE *input = fopen(filepath,"rb");
 /*
  
    algorithm = "c";

    FILE *input = fopen("/Users/Quinnbabe/Documents/lab4/lab4/TestData/input9","r");
*/
	struct basic bibi[99999];
	if((input)==NULL){
		printf("ERROR: can not open input file\n");
		exit(0);
	}
    
    while(!feof(input)){
		char c = fgetc(input);
		if(c == '#'){
			do{
				c = fgetc(input);
			}while(c != '\n');
		}
		else if(c == '\n'){
            
        }
		else{
            fseek(input, -1L, SEEK_CUR);
            bibi[il].index = il;
			fscanf(input," %d ",&bibi[il].at);
            fscanf(input," %d ",&bibi[il].track);
            bibi[il].inq=0;
            il=il+1;
		}
    }
    fclose(input);
    
    struct h hh;
    struct q q1;
    struct q q2;
    int newIn = 1;
   
    struct scdl sch, *schP, fifo, *fifoP, sstf, *sstfP, scan, *scanP, cscan, *cscanP,fscan, *fscanP;
    
    fifo.sq = fsq;
    sstf.sq = sstfSq;
    scan.sq = scanSq;
    cscan.sq = cscanSq;
    fscan.sq = fscanSq;
    
    schP = &sch;
    fifoP = &fifo;
    sstfP = &sstf;
    scanP = &scan;
    cscanP = &cscan;
    fscanP = &fscan;
    
    if(algorithm == "i"){schP = (struct scdl*) fifoP;}
    else if(algorithm == "j"){schP = (struct scdl*) sstfP;}
    else if(algorithm == "s"){schP = (struct scdl*) scanP;}
    else if(algorithm == "c"){schP = (struct scdl*) cscanP;}
    else if(algorithm == "f"){schP = (struct scdl*) fscanP;}
    hh.dir = 1;
    hh.ctr = 0;
    hh.etr = 0;
   
    q1.l = 0;
    q2.l = 0;
    fscanQ = 0; //1st que to add, 2nd que to run
    //bool check;
    while(fi<il){
        //cout<<1<<endl;
        /*
        check=true;
        for(int i=0;i<il;i++){
            if(bibi[i].et==0){
                check=false;
                break;
            }
        }
        if(check)
            break;
         */
        if(algorithm != "f"){
            for(int i=0; i<il;i++){
                if(gt == bibi[i].at && bibi[i].inq==0){
                    faq(&q1, &bibi[i]);
                    bibi[i].inq=1;
                }
            }
        }else{
            if(fscanQ == 0){
                for(int i=0; i<il;i++){
                    if(gt == bibi[i].at && bibi[i].inq==0){
                        faq(&q1, &bibi[i]);
                        bibi[i].inq=1;
                        //if(bibi[i].at==17191)
                          //  cout<<"331 enter queue 1"<<endl;
                    }
                }
            }
            else if(fscanQ == 1){
                for(int i=0; i<il;i++){
                    if(gt == bibi[i].at && bibi[i].inq==0){
                        faq(&q2, &bibi[i]);
                        bibi[i].inq=1;
                        //if(bibi[i].at==17191)
                         //   cout<<"331 enter queue 2"<<endl;
                    }
                }
            }
        }
        //cout<<2<<endl;
        if(algorithm == "f"){
            if(fscanQ == 1){
                if(hh.ctr == hh.etr && q1.l > 0 && newIn == 0){
                    newIn = 1;
                    fi =fi+1;
                    q1.bi[0]->et = gt;
                    q1.bi[0]->tat = gt - q1.bi[0]->at;
                    rq(&q1);
                    
                }
                if(q1.l == 0){
                    fscanQ = 0;
                    hh.dir = 1;
                    //cout<<"turn"<<endl;
                }
            }
            else{
                if(hh.ctr == hh.etr && q2.l > 0 && newIn == 0){
                    newIn = 1;
                    fi =fi+1;
                    q2.bi[0]->et = gt;
                    q2.bi[0]->tat = gt - q2.bi[0]->at;
                                        rq(&q2);
                }
                if(q2.l == 0){
                    fscanQ = 1;
                    hh.dir = 1;
                    //cout<<"turn"<<endl;
                }
            }
        }
        else{
            if(hh.ctr == hh.etr && q1.l > 0 && newIn == 0){
                fi =fi+1;
                newIn = 1;
                q1.bi[0]->et = gt;
                q1.bi[0]->tat = gt - q1.bi[0]->at;
                rq(&q1);
            }
            
        }
        //cout<<3<<endl;
        if(algorithm == "f"){
            if(q1.l == 0){
                fscanQ = 0;
                //cout<<"turn"<<endl;
            }
            else if(q2.l == 0){
                fscanQ = 1;
                //cout<<"turn"<<endl;
            }
            if(newIn == 1 && fscanQ == 1 && q1.l>0){
                fscanSq(&q1,&hh);
                hh.etr = q1.bi[0]->track;
                newIn = 0 ;
                q1.bi[0]->st = gt;
            }
            else if(newIn == 1 && fscanQ == 0 && q2.l>0){
                fscanSq(&q2,&hh);
                hh.etr = q2.bi[0]->track;
                newIn = 0 ;
                q2.bi[0]->st = gt;
         }
        }
        else{
            if(newIn == 1 && q1.l>0){
                for(int i=0;i<q1.l;i++){q1.bi[i]->pl = abs(q1.bi[i]->track - hh.ctr);}
                
                schP->sq(&q1,&hh);
                hh.etr = q1.bi[0]->track;
                newIn = 0 ;
                q1.bi[0]->st = gt;         }
        }
        //cout<<4<<" "<<fi<<" "<<il<<endl;
        if(mh(&hh)==1){
            tot_movement = tot_movement+1;
        }
        gt=gt+1;
        if(tb == 1){
            gt = gt-1;
            tb = 0;
        }
    }
    
    gt = gt-1;
    total_time = gt;
    double twt = 0;
    for(int i=0;i<il;i++){
        avg_turnaround = avg_turnaround + bibi[i].tat;
        bibi[i].wt = bibi[i].st - bibi[i].at;;
        twt = twt+ bibi[i].wt;
        
        if(max_waittime < bibi[i].wt){max_waittime = bibi[i].wt;}
    }
    avg_turnaround = avg_turnaround / il;
    avg_waittime = twt / il;
    printf("SUM: %d %d %.2lf %.2lf %d\n",
           total_time,
           tot_movement,
           avg_turnaround,
           avg_waittime,
           max_waittime);
    
    return 0;
}


