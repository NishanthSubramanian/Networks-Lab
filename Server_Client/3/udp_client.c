#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define MAX 100
#define PORT 8088
#define MAXLINE 1024

int main()
{
    int sockfd;
    char buff[MAXLINE];
    struct sockaddr_in servaddr;
    char hello[MAX];
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    int n, len;
    while (1)
    {
        bzero(hello, sizeof(hello));
        printf("Client : ");
        gets(hello);
        // scanf("%s",hello);
        sendto(sockfd, (const char *)hello, strlen(hello),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));

        n = recvfrom(sockfd, (char *)buff, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                     &len);
        buff[n] = '\0';
        printf("Server : %s\n", buff);
    }
    close(sockfd);
    return 0;
}
