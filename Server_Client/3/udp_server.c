#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define MAX 80
#define PORT 8088
#define SA struct sockaddr
#define MAXLINE 1024

int main()
{
    int sockfd, connfd, len;
    char buff[MAXLINE];
    char hello[MAX];
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));
    bzero(&buff, sizeof(buff));
    int n;
    while (1)
    {
        
        n = recvfrom(sockfd, (char *)buff, sizeof(buff), MSG_WAITALL, (SA *)&cli, &len);
        buff[n] = '\0';
        printf("Client : %s\n", buff);
        bzero(hello, sizeof(hello));
        printf("Server : ");
        gets(hello);
        // scanf("%s",hello);
        sendto(sockfd, (const char *)hello, strlen(hello),
               MSG_CONFIRM, (const struct sockaddr *)&cli,
               len);
    }
}