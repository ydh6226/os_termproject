#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SIZE sizeof(struct sockaddr_in)

#define MAX_FILE_NUMBER 15 //number of file 
#define MAX_FILE_NAME 20//length of filename
#define MAX_BUFF_SIZE 1024

#define SERVER_IP "127.0.0.1"
#define PORT_NUMBER 5000

void main(int argc, char **argv)
{
    int sockfd;

    int menu;

    //menu1
    char fileList[MAX_FILE_NUMBER][MAX_FILE_NAME];
    int fileCount;

    //menu2
    int filedes;
    int readSize;
    char fileName[MAX_FILE_NAME];
    char fileBuff[MAX_BUFF_SIZE];
    char filepath[MAX_FILE_NAME+10];
   

    struct sockaddr_in server = {AF_INET, htons(PORT_NUMBER)};

    server.sin_addr.s_addr = inet_addr(SERVER_IP);  

    while(1){
            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            perror("socket() fail");
            exit(1);
        }

        if (connect(sockfd, (struct sockaddr *)&server, SIZE) == -1){
            perror("connect() fail");
            exit(1);
        }
        // printf("Input menu : ");
        // scanf("%d",menu);
        menu=2;

        menu=htonl(menu);
        send(sockfd,&menu,sizeof(int),0);
        menu=ntohl(menu);
        switch(menu){
            case 1:
                memset(fileList,0x00,MAX_FILE_NUMBER*MAX_FILE_NAME);
                recv(sockfd,fileList,sizeof(fileList),0);

                for(int i=0;i<MAX_FILE_NUMBER;i++){
                    if(fileList[i][0]==0){
                        break;
                    }
                    printf("%s\n",fileList[i]);
                }
                break;
            case 2:
                printf("Input Filename : ");
                scanf("%s",fileName);
                fflush(stdin);

                send(sockfd,fileName,strlen(fileName),0);
                readSize=recv(sockfd,fileBuff,MAX_BUFF_SIZE,0);

                if(strlen(fileBuff)==0){
                    printf("File does not exist\n");
                    break;
                }

                strcpy(filepath,"./download/");
                strcat(filepath,fileName);
                if((filedes=open(filepath,O_CREAT|O_EXCL|O_WRONLY,0644))==-1){
                        perror("open() fail");
                        exit(1);
                }

                while (1){
                    write(filedes,fileBuff,readSize);

                    memset(fileBuff, 0x00, MAX_BUFF_SIZE);
                    readSize=recv(sockfd,fileBuff,MAX_BUFF_SIZE,0);

                    if(readSize==0){
                        printf("End of file download\n");
                        break;
                    }
                }
                break;
            default:
                break;          
        }
        close(sockfd);
    }
}