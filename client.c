#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>


int main(int argc , char *argv[])
{
    srand(time(NULL));
    int randNum = rand() % 65534 + 1;
    int portNum;
    if(randNum < 1024){
        randNum += 1024 + (rand() % 10000 + 1);
    }

    portNum = randNum;

    printf("port number is %d\n", portNum);

    int normalFlag = 0, priorityFlag = 0;  //two connection flag

    fd_set master_set, read_set;
    int maxfd;

    //socket bulid
    int sockfd = 0, listenfd = 0, prifd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1 || listenfd == -1){
        perror("socket()");
    }

    struct sockaddr_in serverInfo, priInfo;
    bzero(&serverInfo, sizeof(serverInfo));
    bzero(&serverInfo, sizeof(priInfo));

    serverInfo.sin_family = PF_INET;
    priInfo.sin_family = PF_INET;

    //localhost
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(8700);

    priInfo.sin_addr.s_addr = INADDR_ANY;
    priInfo.sin_port = htons(portNum);

    bind(listenfd,(struct sockaddr *)&priInfo,sizeof(priInfo));

    listen(listenfd, 5);
    FD_ZERO(&master_set);
    FD_SET(listenfd, &master_set);
    FD_SET(sockfd, &master_set);

    maxfd = listenfd; //> listenfd ? sockfd : listenfd;

    int err = connect(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    if(err == -1){
        perror("connect()");
        exit(0);
    }
    else if(err == 0){
        printf("connect success\n");
        normalFlag = 1;
    }

    char input1[256];
    char input2[256];
    char recMessage[100];;
    if(send(sockfd, &portNum, sizeof(portNum), 0) == -1){
        perror("recv():");
    }

    memset(input1, 0, sizeof(input1));
    memset(input2, 0, sizeof(input2));
    memset(recMessage, 0, sizeof(recMessage));

    while(1){
        read_set = master_set;
        struct timeval time;
        time.tv_sec = 1;
        time.tv_usec = 0;

        int s;
        if((s = select(maxfd + 1, &read_set, NULL, NULL, &time)) <= 0){
            if(s == 0){
                if(normalFlag == 1){
                    printf("input(normal) > ");
                    fgets(input1, 256 , stdin);
                }

                if(priorityFlag == 1){
                    printf("input(priority) > ");
                    fgets(input2, 256 , stdin);
                }

                if(input1[strlen(input1) - 1] == '\n'){
                    input1[strlen(input1) - 1] = '\0';
                }
                if(input2[strlen(input2) - 1] == '\n'){
                    input2[strlen(input2) - 1] = '\0';
                }

                if(strcmp(input1, "exit") == 0){
                    send(sockfd, input1, sizeof(input1), 0);
                    close(sockfd);
                    FD_CLR(sockfd, &master_set);
                    normalFlag = 0;
                }
                else if(strlen(input1) >= 1 && input1[0] != '\0'){
                    send(sockfd, input1, sizeof(input1), 0);
                }

                if(strcmp(input2, "exit") == 0){
                    send(prifd, input2, sizeof(input2), 0);
                    close(prifd);
                    FD_CLR(prifd, &master_set);
                    priorityFlag = 0;
                }
                else if(strlen(input2) >= 1 && input2[0] != '\0'){
                    send(prifd, input2, sizeof(input2), 0);
                }

                // if two socket both close, leave the loop
                if(normalFlag == 0 && priorityFlag ==0){
                    break;
                }

                memset(input1, 0, sizeof(input1));
                memset(input2, 0, sizeof(input2));
            }
            else{
                perror("select()");
                exit(0);
            }
        }
        else{
            for(int i = 0; i <= maxfd; i++){
                if(FD_ISSET(i, &read_set)){
                    memset(recMessage, 0, sizeof(recMessage));

                    if(i == listenfd){
                        struct sockaddr_in priAddr;

                        socklen_t priLen = sizeof(priAddr);

                        prifd = accept(listenfd, (struct sockaddr *)&priAddr, &priLen);
                        if(prifd == -1){
                            perror("accept()");
                            exit(0);
                        }
                        else{
                            printf("Accept priority socket \n");
                            priorityFlag = 1;

                            FD_SET(prifd, &master_set);
                            if(prifd > maxfd){
                                maxfd = prifd;
                            }
                        }
                    }
                    else{
                        if(recv(i, recMessage, sizeof(recMessage), 0) <= 0){
                            perror("recv()");
                        }

                        if(strlen(recMessage) > 0){
                            printf("Server: %s\n", recMessage);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
