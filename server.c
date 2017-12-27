#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    fd_set activefd_set;
    int maxfd;

    char buff[256];
    char message[] = {"Give priority data.\n"};
    int sockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        perror("socket()");
    }

    struct sockaddr_in serverInfo;
    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(8700);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd, 5);

    FD_ZERO(&activefd_set);
    FD_SET(sockfd, &activefd_set);
    maxfd = sockfd;

    while(1){
        int ret;
        struct timeval tv;
        fd_set read_fds;

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        read_fds = activefd_set;
        ret = select(maxfd + 1, &read_fds, NULL, NULL, &tv);
        if(ret == -1){
            perror("select()");
            exit(0);
        }
        else if(ret == 0){
            printf("timeout\n");
        }
        else{
            for(int i = 0; i < FD_SETSIZE; i++){
                if(FD_ISSET(i, &read_fds)){

                    if(i == sockfd){
                        struct sockaddr_in clientAddr;
                        int clientfd;

                        socklen_t clientLen = sizeof(clientAddr);

                        clientfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
                        if(clientfd == -1){
                            perror("accept()");
                            exit(0);
                        }
                        else{
                            printf("Accept client\n");

                            FD_SET(clientfd, &activefd_set);
                            if(clientfd > maxfd){
                                maxfd = clientfd;
                            }
                        }
                    }
                    else{
                        memset(buff, 0, sizeof(buff));
                        int r = recv(i, buff, sizeof(buff), 0);
                        if(r == -1){
                            perror("recv()");
                            exit(0);
                        }
                        else if(r == 0){
                            perror("recv()");
                        }
                        else{
                            printf("Receive: msg=[%s]\n", buff);
                            send(i ,message,sizeof(message), MSG_OOB);  //send priority data
                        }
                    }

                    close(i);
                    FD_CLR(i, &activefd_set);
                }
            }
        }
    }
    return 0;
}
