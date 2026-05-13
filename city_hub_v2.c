#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define NMAX 100

void start_monitor(){
    pid_t hub_mon=fork();

    if(hub_mon<0){
        perror("Warning: Error while using the fork function for hub_mon!\n");
        return;
    }

    if(hub_mon==0){
        int fdes[2];

        if(pipe(fdes)==-1){
            perror("Warning: Error while using the pipe function!\n");
            exit(-1);
        }

        pid_t p=fork();
        if(p<0){
            perror("Warning: Error while using the fork function for p!\n");
            exit(-1);
        }


        if(p==0){
            close(fdes[0]);

            dup2(fdes[1],STDOUT_FILENO);
            close(fdes[1]);

            execl("./monitor_modified","monitor_modified",NULL);

            perror("Warning: Error at the execution of the monitor_modified program!\n");
            exit(-1);
        }
        else{
            close(fdes[1]);

            char buff[256];

            int bytes_read;

            while((bytes_read=read(fdes[0],buff,sizeof(buff)-1))>0){
                buff[bytes_read]='\0';

                if(strncmp(buff,"Error",5)==0){
                    printf("\nHUB_MON has intercepted the following error:\n%s",buff);
                }
                else if(strncmp(buff,"Info",4)==0){
                    printf("\nHUB_MON has intercepted the following information: \n%s",buff);
                }
                else if(strncmp(buff,"Event",5)==0){
                    printf("\nHUB_MON has intercepted the following event: \n%s",buff);
                }
                else{
                    printf("\nHUB_MON has intercepted the following unknown message: \n%s",buff);
                }

                fflush(stdout);
            }


            close(fdes[0]);
            exit(0);
        }
    }

    usleep(50000);
}

int calculate_scores(){
    return 1;
}

int main(){
    char input[256];
    while(1){
        printf("\nCITY_HUB - - - Insert command:");

        if(fgets(input,sizeof(input),stdin)==NULL){
            printf("\nCITY_HUB terminal is closing! Goodbye!\n");
            break;
        }

        input[strcspn(input,"\n")]='\0';

        char* command=strtok(input," ");

        if(command==NULL){
            continue;
        }

        if(strcmp(command,"start_monitor")==0){
            start_monitor();
        }
        else if(strcmp(command,"calculate_scores")==0){
            char district_list[NMAX][NMAX];
            char* district_id=strtok(NULL," ");

            int count=0;

            while(district_id!=NULL && count<NMAX){
                strcpy(district_list[count],district_id);
                count++;
                district_id=strtok(NULL," ");
            }

            if(count>0){
                calculate_scores(district_list,count);
            }
            else{
                printf("Warning: Not enough arguments for the calculate_scores command!\n");
            }
        }
        else if(strcmp(command,"exit")==0){
            int monitor_file=open(".monitor_pid",O_RDONLY);

            if(monitor_file!=-1){
                char pid_string[32];

                int bytes_read;

                if((bytes_read=read(monitor_file,pid_string,sizeof(pid_string)-1))>0){
                    pid_string[bytes_read]='\0';

                    int pid_integer=atoi(pid_string);

                    if(pid_integer>0){
                        kill(pid_integer,SIGINT);
                    }
                }
                close(monitor_file);
            }
            usleep(50000);

            printf("\nCITY_HUB terminal is closing! Goodbye!\n");

            break;
        }
        else{
            printf("CITY_HUB terminal has received an unknown command! Try commands like: start_monitor, calculate_scores and exit!\n");
        }
    }

    return 0;
}
