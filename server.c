#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<wait.h>
#include<arpa/inet.h>

#include"file_processing.h"

#define SIZE sizeof(struct sockaddr_in)

#define STORAGE_PORT 5001
#define MAX_BUFF_SIZE 1024
#define BLOCKED_IP "127.0.0.0"

#define STORAGE_DIR "./storage/"

//end child process
void child_handler(int sig);

void main()
{
    int sockfd_listen;
    int sockfd_connect;
    int clientLen=SIZE;

    pid_t pid;

    int menu;

    //menu1
    char fileList[MAX_FILE_NUMBER][MAX_FILE_NAME];
    int fileCount;

    //menu2
    int DoesNotFileExist;
    int filedes;
    int readSize;
    char fileName[MAX_FILE_NAME];
    char filepath[MAX_FILE_NAME+10];
    char fileBuff[MAX_BUFF_SIZE];
    

    struct sockaddr_in server={AF_INET,htons(STORAGE_PORT),INADDR_ANY};
    struct sockaddr_in client;
    memset(&client,0x00,SIZE);
    
    struct sigaction act;

    act.sa_handler=child_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags=SA_RESTART;
    sigaction(SIGCHLD,&act,NULL);

/*  TURN OFF - TIMEWAIT

    int val = 1;    
    if (setsockopt(sockfd_listen, SOL_SOCKET,SO_REUSEADDR, (char *) &val, sizeof val) < 0) {
	perror("setsockopt");
	close(sockfd_listen);
    }
*/

    if((sockfd_listen=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("socket() fail");
        printf("\n");
        exit(1);
    }

    if(bind(sockfd_listen,(struct sockaddr*)&server,SIZE)==-1){
        perror("bind() fail");
        exit(1);
    }

    if(listen(sockfd_listen,5)==-1){
        perror("listen() fail");
        exit(1);
    }

    printf("Waiting for connection...\n");
    
    while(1){
        if((sockfd_connect=accept(sockfd_listen,(struct sockaddr*)&client,&clientLen))==-1){
            perror("accept() failed");
            continue;
        }
        
        if(strcmp(inet_ntoa(client.sin_addr),BLOCKED_IP)==0){
            printf("Blocked IP connection attempt\n");
            close(sockfd_connect); 
            continue; 
        }



        if((pid=fork())==-1){
            perror("fork() fail");
            exit(1);
        }
        else if(pid==0){
            close(sockfd_listen);
        
            recv(sockfd_connect,&menu,sizeof(int),0);
            menu=ntohl(menu);

            switch(menu){
                case 1:
                    readFileList(fileList,&fileCount,STORAGE_DIR);
                    send(sockfd_connect,fileList,fileCount*MAX_FILE_NAME,0);

                    close(sockfd_connect);
                    exit(2); //child process die
                    break;
                case 2:
                    memset(fileName,0x00,sizeof(fileName));
                    if(recv(sockfd_connect,fileName,MAX_FILE_NAME,0)==0){
                        exit(5);
                    }
                    readFileList(fileList,&fileCount,STORAGE_DIR);
                    for(int i=0;i<fileCount;i++){
                        if((DoesNotFileExist=strcmp(fileName,fileList[i]))==0){//File exists
                            break;
                        }
                    }

                    //File does not exist
                    if(DoesNotFileExist!=0){
                        printf("Client requested a file that does not exist on the server\n");
                        memset(fileBuff, 0x00, MAX_BUFF_SIZE);
                        send(sockfd_connect,fileBuff,MAX_BUFF_SIZE,0);

                        close(filedes);
                        close(sockfd_connect);
                        exit(4);
                    }

                    strcpy(filepath,STORAGE_DIR);
                    strcat(filepath,fileName);
                    if((filedes=open(filepath,O_RDONLY,0644))==-1){
                        perror("open() fail");
                        exit(1);
                    }
                    
                    while (1){
                        memset(fileBuff, 0x00, MAX_BUFF_SIZE);

                        readSize=read(filedes,fileBuff,MAX_BUFF_SIZE);
                        send(sockfd_connect,fileBuff,readSize,0);
                        
                        if(readSize==0)
                            break;
                    }
                    
                    close(filedes);
                    close(sockfd_connect);
                    exit(3); //child process die
                    break;
                default:
                    break;
            }
        }

        close(sockfd_connect); //parent process close a connect socket
    }
    close(sockfd_listen);
}


void child_handler(int sig)
{   
    int status;
    pid_t pid;
    while((pid=waitpid(-1,&status,WNOHANG))>0){
        status=status>>8;
        switch(status){
            case 1:
                //system call fail
                break;
            case 2:
                printf("File list transfer ends\n");
                break;
            case 3:
                printf("File transfer ends\n");
                break;
            case 4:
                //Client requested a file that does not exist on the server
                break;
            case 5:
                //Connect but do nothing
                break;
            default:
                break;
        }
    }
}