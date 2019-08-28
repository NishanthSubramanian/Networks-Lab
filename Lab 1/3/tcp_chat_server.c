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
	printf("LEN : %d\n",len);
	connfd = accept(sockfd, (SA *)&cli, &len);

	char buff[MAX];
	int n;
	while(1)
	{
		bzero(buff, MAX);

		read(connfd, buff, sizeof(buff));

		printf("From client: %s\n To client : ", buff);
		bzero(buff, MAX);
		n = 0;
		gets(buff);
		// scanf("%s",buff);

		write(connfd, buff, sizeof(buff));

		if (strncmp("exit", buff, 4) == 0)
		{
			printf("Server Exit...\n");
			break;
		}
	}

	close(sockfd);
}
