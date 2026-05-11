#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMAX 100

void start_monitor(pid_t cthub_pid){
    pid_t p=fork();
    if(p==0){
        int fdes[2]={0,0};
        pipe(fdes);
        pid_t p1=fork();
        if(p1==0){
            close(fdes[0]);
            execve("monitor_reports.c",NULL,NULL);
        }
        else{
            close(fdes[1]);
            while(1){
                printf("Parinte!\n");
                sleep(1);
            }
        }
    }
}

int main(int argc, char* argv[]){
    if(argc<2){
        perror("Warning: Error while receiving commands for the interface!\n");
        return -1;
    }
    else{
        if(strcmp(argv[1],"start_monitor")==0){
            pid_t p=getpid();
            start_monitor(p);
        }
        if(strcmp(argv[1],"calculate_scores")==0){
            if(argc>2){
                char list_of_districts[argc-2][NMAX];
                for(int i=0;i<argc-2;i++){
                    strcpy(list_of_districts[i],argv[i+2]);
                }
                calculate_scores(list_of_districts);
            }
            else{
                perror("Warning: Not enough arguments for the calculate_scores command!\n");
                return -1;
            }
        }
    }
    return 0;
}
