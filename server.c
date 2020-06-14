#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<wait.h>

#include"server_func.h"

#define SIZE sizeof(struct sockaddr_in)

#define PORT_NUMBER 5000

//end child process
void child_handler(int sig);

void main()
{
    int sockfd_listen;
    int sockfd_connect;
    int filedes;

    pid_t pid;

    char fileList[MAX_FILE_NUMBER][MAX_FILE_NAME];
    int fileCount;

    int menu;

    struct sockaddr_in server={AF_INET,htons(PORT_NUMBER),INADDR_ANY};
    struct sigaction act;

    act.sa_handler=child_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags=SA_RESTART;
    sigaction(SIGCHLD,&act,NULL);
/*
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
        if((sockfd_connect=accept(sockfd_listen,NULL,NULL))==-1){
            perror("accept() failed");
            continue;
        }

        if((pid=fork())==-1){
            perror("fork() fail : ");
            exit(1);
        }
        else if(pid==0){
            close(sockfd_listen);
        
            recv(sockfd_connect,&menu,sizeof(int),0);
            menu=ntohl(menu);

            switch(menu){
                case 1:
                    readFileList(fileList,&fileCount);
                    send(sockfd_connect,fileList,fileCount*MAX_FILE_NAME,0);
                    break;
                case 2:
                    break;
                default:
                    break;
            }
            

            exit(2); //child process die
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
            case 2:
                printf("File list transfer ends\n");
                break;
            case 3:
                break;
            default:
                break;
        }
    }
}
