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
        perror("Directory exist already!\n");
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
        perror("Error while creating symbolic link!\n");
        return;
    }
}


int add(const char* district_id, char* user){
    DIR* d=opendir(district_id);
    if(d==NULL){
        create(district_id);
    }
    char path[512];
    snprintf(path,sizeof(path),"%s/reports.dat",district_id);

    int reports=open(path, O_WRONLY | O_APPEND);
    if(reports==-1){
        perror("Error while opening reports.dat!\n");
        closedir(d);
        return -1;
    }

    Report r;
    memset(&r,0,sizeof(r));

    r.ReportID=rand() % 10000;
    strncpy(r.InspectorName,user,sizeof(r.InspectorName)-1);
    r.timestamp=time(NULL);

    printf("Note GPS coordinates of the Report No. %d:\n",r.ReportID);
    printf("Latitude: ");
    scanf("%f",&r.latitude);
    printf("\nLongitude: ");
    scanf("%f",&r.longitude);
    printf("\nIssue category(road/lightning/flooding/fire) of the Report No. %d: ",r.ReportID);
    scanf("%s",r.category);
    printf("\nSeverity(1=minor/2=moderate/3=critical) of the Report No. %d: ",r.ReportID);
    scanf("%d",&r.severity);
    printf("\nDescription of the Report No. %d: ",r.ReportID);
    scanf(" %[^\n]",r.description);
    printf("\n");

    if(write(reports,&r,sizeof(r))==-1){
        perror("Error while writing the report!\n");
        close(reports);
        closedir(d);
        return -1;
    }

    close(reports);
    closedir(d);
    printf("Report No. %d was successfully announced!\n",r.ReportID);
    return 0;
}

int list(const char* district_id){
    DIR* d=opendir(district_id);
    if(d==NULL){
        perror("The district director does not exist! It has no reports!\n");
        return -1;
    }

    char path[512];
    snprintf(path,sizeof(path),"%s/reports.dat",district_id);

    int reports=open(path,O_RDONLY);

    if(reports==-1){
        perror("Error while opening reports.dat!\n");
        closedir(d);
        return -1;
    }

    struct stat st;
    if(stat(path,&st)==0){
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x\n" : "-\n");
    }

    Report r;

    while(read(reports,&r,sizeof(r))>0){

        char* time=ctime(&r.timestamp);
        time[strlen(time)-1]='\0';

        printf("ReportID: %d\n InspectorName: %s\n GPS Coordinates - Latitude: %f - Longitude: %f\n Issue category: %s\n Severity level: %d\n Timestamp: %s\n Description: %s\n", r.ReportID, r.InspectorName, r.latitude, r.longitude, r.category, r.severity, time, r.description);
    }

    close(reports);
    closedir(d);
    return 0;
}


int view(const char* district_id,int report_id){
    DIR* d=opendir(district_id);
    if(d==NULL){
        perror("The district director does not exist! It has no reports!\n");
        return -1;
    }

    char path[512];
    snprintf(path,sizeof(path),"%s/reports.dat",district_id);

    int reports=open(path,O_RDONLY);

    if(reports==-1){
        perror("Error while opening reports.dat!\n");
        closedir(d);
        return -1;
    }

    Report r;

    while(read(reports,&r,sizeof(r))>0){

        if(r.ReportID==report_id){
            char* time=ctime(&r.timestamp);
            time[strlen(time)-1]='\0';

            printf("ReportID: %d\n InspectorName: %s\n GPS Coordinates - Latitude: %f - Longitude: %f\n Issue category: %s\n Severity level: %d\n Timestamp: %s\n Description: %s\n", r.ReportID, r.InspectorName, r.latitude, r.longitude, r.category, r.severity, time, r.description);

            close(reports);
            closedir(d);
            return 0;
        }
    }

    close(reports);
    closedir(d);
    printf("Report No. %d does not exist in this district directory!\n",report_id);
    return 0;
}

int remove_report(const char* district_id,int report_id){
    struct stat dir;
    if(stat(district_id,&dir)==-1 || !S_ISDIR(dir.st_mode)){
        perror("The district director does not exist! It has no reports!\n");
        return -1;
    }

    char path[512];
    snprintf(path,sizeof(path),"%s/reports.dat",district_id);

    int reports=open(path,O_RDWR);

    if(reports==-1){
        perror("Error while opening reports.dat!\n");
        return -1;
    }

    Report r;
    int found=0;
    off_t read_pos=0;
    off_t write_pos=0;

    while(read(reports,&r,sizeof(r))>0){
        if(r.ReportID==report_id){
            found=1;

            read_pos=lseek(reports,0,SEEK_CUR);
            write_pos=read_pos-sizeof(r);

            break;
        }
    }

    if(!found){
        printf("Report No. %d was not found in reports.dat!\n",report_id);
        close(reports);
        return -1;
    }
    else{
        while(1){
            lseek(reports,read_pos,SEEK_SET);

            if(read(reports,&r,sizeof(r))<=0)break;

            read_pos=lseek(reports,0,SEEK_CUR);

            lseek(reports,write_pos,SEEK_SET);
            write(reports,&r,sizeof(r));

            write_pos=lseek(reports,0,SEEK_CUR);
        }

        if(ftruncate(reports,write_pos)==-1){
            printf("Error while truncating reports.dat!\n");
            close(reports);
            return -1;
        }
        else{
            printf("Report No. %d was successfully removed!\n",report_id);
        }
    }

    close(reports);
    return 0;
}

int main(int argc,char* argv[]){
    char* role=NULL;
    char* user=NULL;
    char* command=NULL;
    char* district=NULL;
    int report_id=-1;
    int value=0;
    int condition=0;
    srand(time(NULL));

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
            add(district,user);
        }
        else if(strcmp(argv[i],"--list")==0){
            command="list";
            district=argv[++i];
            list(district);
        }
        else if(strcmp(argv[i],"--view")==0){
            command="view";
            district=argv[++i];
            report_id=atoi(argv[++i]);
            view(district,report_id);
        }
        else if(strcmp(argv[i],"--remove_report")==0){
            command="remove_report";
            district=argv[++i];
            report_id=atoi(argv[++i]);
            remove_report(district,report_id);
        }
        else if(strcmp(argv[i],"--update_threshold")==0){
            command="update_threshold";
            district=argv[++i];
            value=atoi(argv[++i]);
        }
        else if(strcmp(argv[i],"--filter")==0){
            command="filter";
            district=argv[++i];
            condition=atoi(argv[++i]);
        }
    }
    return 0;
}
