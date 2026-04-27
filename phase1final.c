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
        chmod(path,0664);
    }

    snprintf(path,sizeof(path),"%s/district.cfg",district_id);
    int districtCFG=open(path,O_WRONLY | O_CREAT | O_TRUNC,0640);
    if(districtCFG!=-1){
        close(districtCFG);
        chmod(path,0640);
    }

    snprintf(path,sizeof(path),"%s/logged_district",district_id);
    int loggedDistrict=open(path,O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(loggedDistrict!=-1){
        close(loggedDistrict);
        chmod(path,0644);
    }

    char link[256];
    struct stat st;
    snprintf(link,sizeof(link),"active_reports-%s",district_id);
    snprintf(path,sizeof(path),"%s/reports.dat",district_id);

    if(lstat(link,&st)==0){
        unlink(link);
    }

    if(symlink(path,link)==-1){
        perror("Error while creating symbolic link!\n");
        return;
    }
}

int role_verifier(const char* role){
    if(role==NULL){
        return 0;
    }
    else if(strcmp(role,"manager")==0){
        return 1;
    }
    else if(strcmp(role,"inspector")==0){
        return 2;
    }
    return 0;
}

int log_action(const char* district_id, char* role, char* user, char* action){

    if (strcmp(role, "inspector") == 0) {
        printf("Inspectors cannot write to the log file!\n");
        return -1;
    }
    char path[512];

    snprintf(path,sizeof(path),"%s/logged_district",district_id);
    int loggedDistrict=open(path,O_WRONLY | O_APPEND);

    if(loggedDistrict==-1){
        perror("Error while opening logged_district!");
        return -1;
    }

    time_t initial=time(NULL);
    char* time=ctime(&initial);
    time[strlen(time)-1]='\0';

    if(role==NULL || user==NULL){
        perror("Error while passing details for the log!");
        return -1;
    }

    char buff[256];
    int buff_len=snprintf(buff,sizeof(buff),"Time: %s  | Role: %s | User: %s | Action: %s",time,role,user,action);

    write(loggedDistrict,buff,buff_len);
    close(loggedDistrict);
    return 0;
}

int add(const char* district_id, char* user, char* role){
    DIR* d=opendir(district_id);
    if(d==NULL){
        create(district_id);
    }
    char path[512];
    snprintf(path,sizeof(path),"%s/reports.dat",district_id);

    int reports=open(path, O_WRONLY | O_APPEND);
    struct stat st;
    if (stat(path, &st) == 0) {
        if ((st.st_mode & 0777) != 0664) {
            chmod(path, 0664);
        }
    }
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

    char action[256];
    snprintf(action,sizeof(action),"Added: Report No. %d\n",r.ReportID);
    log_action(district_id,role,user,action);

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
        printf("Permissions: ");
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");

        printf(" | Size: %ld bytes | Last modified: %s\n", st.st_size, ctime(&st.st_mtime));
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

int remove_report(const char* district_id,int report_id, char* user, char* role){
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

            char action[256];
            snprintf(action,sizeof(action),"Removed: Report No. %d\n",report_id);
            log_action(district_id,role,user,action);
        }
    }

    close(reports);
    return 0;
}

int update_threshold(const char* district_id, int value, char* user, char* role){
    char path[512];

    snprintf(path,sizeof(path),"%s/district.cfg",district_id);
    struct stat st;
    if (stat(path, &st) == -1)return -1;

    if ((st.st_mode & 0777) != 0640) {
        printf("district.cfg permissions are incorrect (%o). Aborting for security!\n", st.st_mode & 0777);
        return -1;
    }
    int districtCFG=open(path,O_WRONLY | O_TRUNC);

    if(districtCFG==-1){
        perror("Error while opening district.cfg!\n");
        return -1;
    }

    char buff[30];
    int buff_len=snprintf(buff,sizeof(buff),"Severity threshold setted at level %d!\n",value);

    write(districtCFG,buff,buff_len);
    close(districtCFG);

    char action[256];
    snprintf(action,sizeof(action),"Severity threshold updated to: %d\n",value);
    log_action(district_id,role,user,action);

    return 0;
}

int parse_condition(const char* condition, char* type, char* operation, char* value){
    const char* type1=strchr(condition,':');
    if(type1==NULL)return -1;

    const char* operation1=strchr(type1+1,':');
    if(operation1==NULL)return -1;

    int type_len=type1-condition;
    strncpy(type,condition,type_len);
    type[type_len]='\0';

    int operation_len=operation1-(type1+1);
    strncpy(operation,type1+1,operation_len);
    operation[operation_len]='\0';

    strcpy(value,operation1+1);
    return 0;
}

int match_condition(Report* r, char* type, char* operation, char* value){
    if(type==NULL || operation==NULL || value==NULL){
        return -1;
    }

    if(strcmp(type,"severity")==0){
        int numeric_value=atoi(value);
        if(strcmp(operation,"==")==0)return r->severity==numeric_value;
        if(strcmp(operation,"!=")==0)return r->severity!=numeric_value;
        if(strcmp(operation,"<")==0)return r->severity<numeric_value;
        if(strcmp(operation,"<=")==0)return r->severity<=numeric_value;
        if(strcmp(operation,">")==0)return r->severity>numeric_value;
        if(strcmp(operation,">=")==0)return r->severity>=numeric_value;
    }
    if(strcmp(type,"timestamp")==0){
        time_t numeric_value=(time_t)atol(value);
        if(strcmp(operation,"==")==0)return r->timestamp==numeric_value;
        if(strcmp(operation,"!=")==0)return r->timestamp!=numeric_value;
        if(strcmp(operation,"<")==0)return r->timestamp<numeric_value;
        if(strcmp(operation,"<=")==0)return r->timestamp<=numeric_value;
        if(strcmp(operation,">")==0)return r->timestamp>numeric_value;
        if(strcmp(operation,">=")==0)return r->timestamp>=numeric_value;
    }
    if(strcmp(type,"category")==0){
        if(strcmp(operation,"==")==0)return strcmp(r->category,value)==0;
        if(strcmp(operation,"!=")==0)return strcmp(r->category,value)!=0;
    }
    if(strcmp(type,"inspector")==0){
        if(strcmp(operation,"==")==0)return strcmp(r->InspectorName,value)==0;
        if(strcmp(operation,"!=")==0)return strcmp(r->InspectorName,value)!=0;
    }
    return -1;
}

int filter(const char* district_id, char* condition){
    char type[20],operation[3],value[30];

    if(parse_condition(condition,type,operation,value)){
        perror("Error while parsing the condition!\n");
        return -1;
    }

    char path[512];

    snprintf(path,sizeof(path),"%s/reports.dat",district_id);
    int reports=open(path,O_RDONLY);

    if(reports==-1){
        perror("Error while opening reports.dat!\n");
        return -1;
    }

    Report r;
    printf("The results of the filter:\n");

    while(read(reports,&r,sizeof(r))>0){
        if(match_condition(&r,type,operation,value)==1){
            char* time = ctime(&r.timestamp);
            time[strlen(time)-1] = '\0';

            printf("ReportID: %d | InspectorName: %s | Category: %s | Severity: %d | Timestamp: %s | Description: %s\n",
            r.ReportID, r.InspectorName, r.category, r.severity, time, r.description);
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
    char* condition=NULL;
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
        }
        else if(strcmp(argv[i],"--list")==0){
            command="list";
            district=argv[++i];
        }
        else if(strcmp(argv[i],"--view")==0){
            command="view";
            district=argv[++i];
            report_id=atoi(argv[++i]);
        }
        else if(strcmp(argv[i],"--remove_report")==0){
            command="remove_report";
            district=argv[++i];
            report_id=atoi(argv[++i]);
        }
        else if(strcmp(argv[i],"--update_threshold")==0){
            command="update_threshold";
            district=argv[++i];
            value=atoi(argv[++i]);
        }
        else if(strcmp(argv[i],"--filter")==0){
            command="filter";
            district=argv[++i];
            condition=argv[++i];
        }
    }
    if(command!=NULL){
        int role_level=role_verifier(role);

        if(strcmp(command,"add")==0){
            if(role_level>=1){
                add(district,user,role);
            }
            else{
                printf("The add operation can not be done by any citizen!");
            }
        }
        if(strcmp(command,"list")==0){
            if(role_level>=1){
                list(district);
            }
            else{
                printf("The list operation can not be done by any citizen!");
            }
        }
        if(strcmp(command,"view")==0){
            if(role_level>=1){
                view(district,report_id);
            }
            else{
                printf("The view operation can not be done by any citizen!");
            }
        }
        if(strcmp(command,"remove_report")==0){
            if(role_level==1){
                remove_report(district,report_id,user,role);
            }
            else{
                printf("The remove_report operation can not be done by someone who is not the manager!");
            }
        }
        if(strcmp(command,"update_threshold")==0){
            if(role_level==1){
                update_threshold(district,value,user,role);
            }
            else{
                printf("The update_threshold operation can not be done by someone who is not the manager!");
            }
        }
        if(strcmp(command,"filter")==0){
            if(role_level>=1){
                filter(district,condition);
            }
            else{
                printf("The filter operation can not be done by any citizen!");
            }
        }
    }
    return 0;
}
