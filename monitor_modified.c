#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

void handler(int x){
    if(x==SIGINT)
    {
        printf("Event: The SIGINT signal has been seized! The program stopped!\n");
        fflush(stdout);
        unlink(".monitor_pid");
        exit(0);
    }
    if(x==SIGUSR1)
    {
        printf("Event: The SIGUSR1 signal has been seized! A report has been added!\n");
        fflush(stdout);
        return;
    }
    return;
}

int main(){
    int check_mon=open(".monitor_pid",O_RDONLY);

    if(check_mon!=-1){
        char existing_pid[32];

        int bytes_read=read(check_mon,existing_pid,sizeof(existing_pid)-1);

        if(bytes_read>0){
            existing_pid[bytes_read]='\0';

            printf("Error: The monitor is already running, having the %s process ID!\n",existing_pid);
            fflush(stdout);

            close(check_mon);
            exit(1);
        }

        close(check_mon);
    }


    char path[512];
    snprintf(path,sizeof(path),".monitor_pid");

    int monitor=open(path,O_WRONLY | O_CREAT | O_TRUNC, 0664);

    pid_t pid=getpid();

    char buff[32];
    int buff_len=snprintf(buff,sizeof(buff),"%d",pid);

    if(write(monitor,buff,buff_len)<=0){
        perror("Warning: Error while writing the procces id in the monitor_pid file!\n");
        return -1;
    }


    close(monitor);

    printf("Info: Monitor started with the following procces ID: %d!\n",pid);
    fflush(stdout);

    struct sigaction sig;
    memset(&sig,0,sizeof(sig));
    sig.sa_handler=handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags=0;

    sigaction(SIGINT,&sig,NULL);
    sigaction(SIGUSR1,&sig,NULL);

    while(1){
        pause();
    }

    return 0;
}
