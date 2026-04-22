#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

typedef struct Report{
    int ReportID;
    char InspectorName[35];
    float latitude;
    float longitude;
    char category[20];
    int severity;
    time_t timestamp;
    char description[129];
}Report;

void create(const char *district_id){
    char path[512];

    if(mkdir(district_id,0750)==-1){
        perror("Directory for %s exist already!",district_id);
        return;
    }

    snprintf(path,sizeof(path),"%s/reports.dat",district_id);
    int reports=open(path,O_WRONLY | O_CREAT | O_APPEND,0664);
    if(reports!=-1){
        close(reports);
    }

    snprintf(path,sizeof(path),"%s/district.cfg",district_id);
    int districtCFG=open(path,O_WRONLY | O_CREAT | O_TRUNC,0640);
    if(districtCFG!=-1){
        close(districtCFG);
    }

    snprintf(path,sizeof(path),"%s/logged_district",district_id);
    int loggedDistrict=open(path,O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(loggedDistrict!=-1){
        close(loggedDistrict);
    }

    char link[256];
    snprintf(link,sizeof(link),"active_reports-%s",district_id);
    snprintf(path,sizeof(path),"%s/reports.dat",district_id);

    unlink(link);
    if(symlink(path,link)==-1){
        perror("Error while creating symbolic link!");
        return;
    }
}


int add(const char* district_id){
    DIR* d=opendir(district_id);
    if(d==NULL){

    }
}

int main(int argc,char* argv[]){
    char* role=NULL;
    char* user=NULL;
    char* command=NULL;
    char* district=NULL;
    char* report_id=NULL;
    char* value=NULL;
    char* condition=NULL;
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"--role")==0){
            role=argv[++i];
        }
        else if(strcmp(argv[i],"--user")==0){
            user=argv[++i];
        }
        else if(strcmp(argv[i],"--add")==0){
            command="add";
            district=argv[++i];
        }
        else if(strcmp(argv[i],"--list")==0){
            command="list";
            district=argv[++i];
        }
        else if(strcmp(argv[i],"--view")==0){
            command="view";
            district=argv[++i];
            report_id=argv[++i];
        }
        else if(strcmp(argv[i],"--remove_report")==0){
            command="remove_report";
            district=argv[++i];
            report_id=argv[++i];
        }
        else if(strcmp(argv[i],"--update_threshold")==0){
            command="update_threshold";
            district=argv[++i];
            value=argv[++i];
        }
        else if(strcmp(argv[i],"--filter")==0){
            command="filter";
            district=argv[++i];
            condition=argv[++i];
        }
    }
    return 0;
}
