#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

struct user{
    int normalId;
    int priorityId;
    int port;
    struct user* next;
};

struct user* rootUser; //to record userdata

int connectToClient(int id, int port);

void removeId(int id);

fd_set activefd_set;
int maxfd;

int portNum = 8700; //port number is 8700
char* address = "127.0.0.1"; //address is 127.0.0.1

int main(int argc, char *argv[])
{
    // init rootUser
    rootUser = (struct user *)malloc(sizeof(struct user));
    rootUser->normalId = -1;
    rootUser->priorityId = -1;
    rootUser->port = -1;
    rootUser->next = NULL;

    char buff[256];
    char message[] = {"Give priority data.\n"};
    int listenfd = 0;
    //create socket
    listenfd = socket(AF_INET , SOCK_STREAM , 0);

    if (listenfd == -1){
        perror("socket()");
    }

    struct sockaddr_in serverInfo;
    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(address);  //only for 127.0.0.1
    serverInfo.sin_port = htons(portNum);  //only for port 8700

    int option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // bind it
    bind(listenfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));

    // listen
    listen(listenfd, 5);

    FD_ZERO(&activefd_set);
    FD_SET(listenfd, &activefd_set);  //add listenfd to set
    maxfd = listenfd;

    while(1){
        fd_set read_fds;

        read_fds = activefd_set;
        if(select(maxfd + 1, &read_fds, NULL, NULL, NULL) <= 0){
            perror("select()");
            exit(0);
        }
        else{
            for(int i = 0; i <= maxfd; i++){
                if(FD_ISSET(i, &read_fds)){

                    if(i == listenfd){
                        struct sockaddr_in clientAddr;
                        int clientfd;

                        socklen_t clientLen = sizeof(clientAddr);

                        clientfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientLen);
                        if(clientfd == -1){
                            perror("accept()");
                            exit(0);
                        }
                        else{
                            printf("Accept client\n");
                            printf("clientdf: %d\n", clientfd);

                            FD_SET(clientfd, &activefd_set);
                            if(clientfd > maxfd){
                                maxfd = clientfd;
                            }
                        }
                    }
                    else{
                        struct user* u = rootUser->next;
                        int normalFlag = 0;
                        int priorityFlag = 0;
                        while(u != NULL){
                            if(u->normalId == i || u->priorityId == i){
                                if(u->normalId == i){normalFlag = 1;}
                                else if(u->priorityId == i){priorityFlag = 1;}

                                //printf("normalId: %d, priorityId: %d, listensd: %d\n", u->normalId, u->priorityId, listenfd);

                                break;
                            }
                            u = u->next;
                        }

                        // process the data from client that has been connected
                        if(normalFlag == 1 || priorityFlag == 1){
                            //printf("normalFlag: %d, priorityFlag: %d\n", normalFlag, priorityFlag);
                            memset(buff, 0, sizeof(buff));
                            char sendData[256];
                            memset(sendData, 0, sizeof(sendData));
                            int rec;
                            if((rec = recv(i, buff, sizeof(buff), 0)) <= 0){
                                perror("recv() rec");
                            }

                            if(normalFlag == 1){
                                memcpy(sendData, "[normal]You said: ", 18);
                            }
                            else if(priorityFlag == 1){
                                memcpy(sendData, "[priotity]You said: ", 20);
                            }

                            if(strcmp(buff, "exit") == 0){
                                close(i);
                                FD_CLR(i, &activefd_set);
                                removeId(i);
                            }
                            else{

                                strcat(sendData, buff); //combine
                                printf("output: %s\n", sendData);
                                send(i, sendData, sizeof(sendData), 0);
                            }
                        }
                        // process the data from client that has not been connected
                        else if(normalFlag == 0 && priorityFlag == 0){
                            int port;
                            if(recv(i, &port, sizeof(port), 0) <= 0){
                                perror("recv() port");
                            }
                            else{
                                printf("Receive: port = %d, user = %d\n", port, i);
                                connectToClient(i, port);
                            }
                        }
                    }

                    /*close(i);
                    FD_CLR(i, &activefd_set);*/
                }
            }
        }
    }
    return 0;
}

int connectToClient(int id, int port){
    struct user* newUser = (struct user *)malloc(sizeof(struct user));

    int sockfd = socket(AF_INET , SOCK_STREAM , 0);

    newUser->normalId = id;
    newUser->port = port;
    newUser->priorityId = sockfd;
    newUser->next = NULL;

    if (sockfd == -1){
        perror("socket()");
    }

    struct sockaddr_in clientInfo;
    bzero(&clientInfo, sizeof(clientInfo));
    clientInfo.sin_family = PF_INET;

    //localhost test
    clientInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientInfo.sin_port = htons(port);


    int err = connect(sockfd,(struct sockaddr *)&clientInfo,sizeof(clientInfo));
    if(err == -1){
        perror("connect()");
        exit(0);
    }
    else if(err == 0){
        printf("connect success\n");
    }

    send(sockfd, "Hi, this is the priority socket!", 32, 0);

    struct user* u = rootUser->next;
    struct user* pre = rootUser;
    while(u != NULL){
        pre = u;
        u = u->next;
    }

    pre->next = newUser;

    FD_SET(sockfd, &activefd_set);
    if(sockfd > maxfd){
        maxfd = sockfd;
    }

    printf("new connection, normal socket: %d, priority socket: %d, client port: %d, priority port: %d\n", newUser->normalId, newUser->priorityId, portNum, newUser->port);

    /*struct user* temp = rootUser->next;
    while(temp != NULL){
        printf("normalId: %d, priorityId: %d, port: %d\n", temp->normalId, temp->priorityId, port);
        temp = temp->next;
    }*/
}

void removeId(int id){
    struct user* u = rootUser->next;
    struct user* pre = rootUser;

    while(u != NULL){
        if(u->normalId == id){
            u->normalId = -1;
            printf("normalId: %d leave\n", id);
        }
        if(u->priorityId == id){
            u->priorityId = -1;
            printf("priorityId: %d leave\n", id);
        }

        if(u->normalId == -1 && u->priorityId == -1){
            pre->next = u->next;
            printf("A user out\n");
            free(u);
        }

        pre = u;
        u = u->next;
    }
}
