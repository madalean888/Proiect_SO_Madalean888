#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

void sa_handler(int x){
    if(x==SIGINT)
    {
        printf("\nThe SIGINT signal has been seized! The program stopped!\n");
        unlink(".monitor_pid");
        exit(0);
    }
    if(x==SIGUSR1)
    {
        printf("\nThe SIGUSR1 signal has been seized! A report has been added!\n");
        return;
    }
    return;
}

int main(){

    char path[512];
    snprintf(path,sizeof(path),".monitor_pid");

    int monitor=open(path,O_WRONLY | O_CREAT | O_TRUNC, 0664);

    pid_t pid=getpid();

    char buff[32];
    int buff_len=snprintf(buff,sizeof(buff),"%d",pid);

    if(write(monitor,buff,buff_len)<=0){
        perror("Error while writing the procces id in the monitor_pid file!\n");
        return -1;
    }


    close(monitor);

    printf("Monitor started with the following procces ID: %d!\n",pid);

    struct sigaction sig;
    memset(&sig,0,sizeof(sig));
    sig.sa_handler=sa_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags=0;

    sigaction(SIGINT,&sig,NULL);
    sigaction(SIGUSR1,&sig,NULL);

    while(1){
        pause();
    }

    return 0;
}
