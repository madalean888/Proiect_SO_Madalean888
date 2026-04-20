#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

typedef struct File_Type{
    char file_type[10];
    char name[20];
    bool created;
}Directory;

void create(Directory *directory_name){

    struct stat buff;
    if(stat(directory_name->name,&buff)==0)
    {
        printf("The directory already exist!\n");
        return;
    }
    else
    {
        directory_name->created=true;
        if(strcmp(directory_name->file_type,"Directory")==0){
            mkdir(directory_name->name,0750)
        }
        if(strcmp(directory_name->file_type,"File")==0 && strcmp(directory_name->name,"district.cfg")==0){
            mkdir(directory_name->name,0640)
        }
        printf("The directory %s has been created!\n",directory_name->name);
    }

    printf("Error creating directory!\n")
    directory_name->created=false;
    return;
}


void create_file(FILE *)

int add(const char* district_id){
    DIR* d=opendir(district_id);
    if(d==NULL){

    }
}

int main(int argv,char* argv[]){
    return 0;
}
