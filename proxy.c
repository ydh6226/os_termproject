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
    char fileName[MAX_FILE_NAME];    
   

    struct sockaddr_in server = {AF_INET, htons(PORT_NUMBER)};

    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket() fail");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&server, SIZE) == -1){
        perror("connect() fail");
        exit(1);
    }
    
    menu=2;

    while(1){
        menu=htonl(menu);
        send(sockfd,&menu,sizeof(int),0);

        switch(menu){
            case 1:
                memset(fileList,0,MAX_FILE_NUMBER*MAX_FILE_NAME);
                recv(sockfd,fileList,sizeof(fileList),0);

                for(int i=0;i<MAX_FILE_NUMBER;i++){
                    if(fileList[i][0]==0){
                        break;
                    }
                    printf("%s\n",fileList[i]);
                }
                close(sockfd);
                break;
            case 2:{
                printf("Input Filename : ");
                scanf
            }

        }


    }
    
}