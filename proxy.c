#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<fcntl.h>
#include<wait.h>

#define SIZE sizeof(struct sockaddr_in)

#define MAX_FILE_NUMBER 15 //number of file 
#define MAX_FILE_NAME 20//length of filename
#define MAX_BUFF_SIZE 1024

#define STORAGE_IP "127.0.0.1"
#define STORAGE_PORT 5000

#define PROXY_PORT 6000

//end child process
void child_handler(int sig);

void main(int argc, char **argv)
{   
    int sockfd_listen;//server
    int sockfd_connect;//server
    int sockfd;//client
    int menu;

    pid_t pid;

    //menu1 : request file list
    char fileList[MAX_FILE_NUMBER][MAX_FILE_NAME];
    int fileCount=0;

    //menu2 : request file
    int filedes;
    int readSize;
    char fileName[MAX_FILE_NAME];
    char fileBuff[MAX_BUFF_SIZE];
    char filepath[MAX_FILE_NAME+10];
   
    //storage server address
    struct sockaddr_in server = {AF_INET, htons(STORAGE_PORT)};
    server.sin_addr.s_addr = inet_addr(STORAGE_IP);  

    //proxy address
    struct sockaddr_in proxy={AF_INET,htons(PROXY_PORT),INADDR_ANY};
    struct sigaction act;

    act.sa_handler=child_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags=SA_RESTART;
    sigaction(SIGCHLD,&act,NULL);

    //make socket for accepting client
    if((sockfd_listen=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("socket() fail");
        printf("\n");
        exit(1);
    }

    if(bind(sockfd_listen,(struct sockaddr*)&proxy,SIZE)==-1){
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
            perror("fork() fail");
            exit(1);
        }
        else if(pid==0){
                close(sockfd_listen);

                recv(sockfd_connect,&menu,sizeof(int),0);

                //make socket for connecting storage server
                if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
                    perror("socket() fail");
                    exit(1);
                }

                if (connect(sockfd, (struct sockaddr *)&server, SIZE) == -1){
                    perror("connect() fail");
                    exit(1);
                }

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
                            fileCount++;
                        }

                        send(sockfd_connect,fileList,fileCount*MAX_FILE_NAME,0);

                        close(sockfd);
                        close(sockfd_connect);
                        exit(2);
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

                        close(sockfd);
                        close(sockfd_connect);
                        exit(3);
                        break;
                    default:
                        break;          
                }
        }
        close(sockfd_connect);//parent process close a connect socket
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
            default:
                break;
        }
    }
}
