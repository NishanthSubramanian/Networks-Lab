#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 80
#define PORT 8085
#define SA struct sockaddr

int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

	listen(sockfd, 5);
	len = sizeof(cli);

	connfd = accept(sockfd, (SA *)&cli, &len);

	char *buff = "Greetings";
	write(connfd, buff, sizeof(buff));

	close(sockfd);
}
