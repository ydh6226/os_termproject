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
#define STORAGE_PORT 5001

#define PROXY_IP "127.0.0.1"
#define PROXY_PORT 6000

#define DOWNLOAD_DIR "./download/"

void child_handler(int sig); //end child process
void USER_handler(int sig); //waiting printf() of child process

void main(int argc, char **argv)
{   
    int sockfd;
    int menu=0;
    int proxyOn=0;
    char buff[MAX_BUFF_SIZE];

    pid_t pid;

    //menu1 : request file list
    char fileList[MAX_FILE_NUMBER][MAX_FILE_NAME];
    int fileCount;

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
    struct sockaddr_in proxy = {AF_INET, htons(PROXY_PORT)};
    server.sin_addr.s_addr = inet_addr(PROXY_IP);

    sigset_t set1;
    sigemptyset(&set1);
    sigaddset(&set1,SIGCHLD);

    sigset_t set2;
    sigemptyset(&set2);
    sigaddset(&set2,SIGUSR1);

    struct sigaction act;
    act.sa_handler=child_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags=SA_RESTART;
    sigaction(SIGCHLD,&act,NULL);

    struct sigaction act2;
    act2.sa_handler=USER_handler;
    sigfillset(&act2.sa_mask);
    sigaction(SIGUSR1,&act2,NULL);

    while(1){
        sigprocmask(SIG_SETMASK,&set1,NULL); //block SIGCHLD, don't block SIGUSR1

        if((menu==1)||(menu==2))
            pause();//waiting printf() of child process

        printf("\n==============================\n");
        printf("1. File list\n");
        printf("2. Request File\n");
        printf("3. Rroxy ON/OFF(default : OFF)\n");

        do{
            printf("\nInput menu : ");
            if(scanf("%d",&menu)<=0){
                printf("\nPlease enter the correct menu\n");
                fgets(buff,MAX_BUFF_SIZE,stdin);//exception handling
                menu=0;
            }
        }while(!((menu>=1)&&(menu<=3)));

        if(menu==3){
            if(proxyOn==0){
                proxyOn=1;
                printf("\nProxy ON\n");
            }
            else{
                proxyOn=0;
                printf("\nProxy OFF\n");
            }
            continue;
        }

        sigprocmask(SIG_SETMASK,&set2,NULL); //don't block SIGCHLD, block SIGUSR1
        fflush(stdin);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket() fail");
        exit(1);
        }

        if(proxyOn==1){
            if (connect(sockfd, (struct sockaddr *)&proxy, SIZE) == -1){
                perror("connect() fail");
                exit(1);
            }
        }
        else{
            if (connect(sockfd, (struct sockaddr *)&server, SIZE) == -1){
                perror("connect() fail");
                exit(1);
            }
        }

        if((pid=fork())==-1){
            perror("fork() fail");
            exit(1);
        }
        else if(pid==0){
            menu=htonl(menu);
            send(sockfd,&menu,sizeof(int),0);

            menu=ntohl(menu);
            switch(menu){
                case 1:
                    memset(fileList,0x00,MAX_FILE_NUMBER*MAX_FILE_NAME);
                    recv(sockfd,fileList,sizeof(fileList),0);

                    printf("\n<File list>\n");
                    for(int i=0;i<MAX_FILE_NUMBER;i++){
                        if(fileList[i][0]==0){
                            break;
                        }
                        printf("%s\n",fileList[i]);
                    }
                    
                    close(sockfd);
                    
                    usleep(10);
                    kill(getppid(),SIGUSR1); //end of printf()
                    exit(2);
                    break;
                case 2:
                    printf("\nInput Filename : ");
                    scanf("%s",fileName);
                    fgets(buff,MAX_BUFF_SIZE,stdin);

                    send(sockfd,fileName,strlen(fileName),0);
                    readSize=recv(sockfd,fileBuff,MAX_BUFF_SIZE,0);

                    if(strlen(fileBuff)==0){
                        printf("\nFile does not exist\n");
                        kill(getppid(),SIGUSR1); //end of printf()
                        break;
                    }

                    kill(getppid(),SIGUSR1); //end of printf()

                    strcpy(filepath,DOWNLOAD_DIR);
                    strcat(filepath,fileName);
                    unlink(filepath);
                    if((filedes=open(filepath,O_CREAT|O_EXCL|O_WRONLY,0644))==-1){
                            perror("\nopen() fail");
                            exit(1);
                    }

                    while (1){
                        write(filedes,fileBuff,readSize);

                        memset(fileBuff, 0x00, MAX_BUFF_SIZE);
                        readSize=recv(sockfd,fileBuff,MAX_BUFF_SIZE,0);

                        if(readSize==0){
                            exit(3);
                        }
                    }

                    close(sockfd);
                    exit(1);
                    break;
                default:
                    break;          
            }
        }
        close(sockfd);
    }
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
                //File list reception complete
                break;
            case 3:
                printf("\n------------------------------\n");
                printf("File download complete\n");
                printf("------------------------------\n");
                break;
            default:
                break;
        }
    }
}

void USER_handler(int sig)
{
    //do nothing
}