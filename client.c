#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc , char *argv[])
{

    //socket bulid
    int sockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1){
        perror("socket()");
    }

    struct sockaddr_in serverInfo;
    bzero(&serverInfo, sizeof(serverInfo));
    serverInfo.sin_family = PF_INET;

    //localhost test
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(8700);


    int err = connect(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    if(err == -1){
        perror("connect()");
        exit(0);
    }
    else if(err == 0){
        perror("connect()");
    }


    //Send a message to server
    char message[] = {"Give normal data."};
    char recMessage[100] = {};
    int s = send(sockfd, message, sizeof(message), 0);
    if(s == -1){
        perror("recv():");
    }

    int flag = recv(sockfd, recMessage, sizeof(recMessage), 0);
    if(flag == -1){
        perror("recv():");
    }
    else if(flag == 0){
        perror("recv():");
    }
    else{
        printf("recv!\n");
    }

    printf("%s", recMessage);
    printf("close Socket\n");
    close(sockfd);

    return 0;
}
