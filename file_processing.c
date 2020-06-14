#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<dirent.h>
#include<unistd.h>

#include"file_processing.h"

void readFileList(char fileList[][MAX_FILE_NAME],int *fileCount,const char *dirName)
{
    DIR *dirp;
    struct dirent *dentry;
 
    memset(fileList,0,MAX_FILE_NUMBER*MAX_FILE_NAME); 
    *fileCount=0;    

    if((dirp=opendir(dirName))==NULL){
        perror("opendir() fail");
    }

    while(dentry=readdir(dirp)){
        if(dentry->d_ino!=0){
            char fileName[20];
            strcpy(fileName,dentry->d_name);

            if((strcmp(fileName,".")!=0)&&(strcmp(fileName,"..")!=0)){
                strcpy(fileList[(*fileCount)++],fileName);
            }
        }
    }
    closedir(dirp);
}