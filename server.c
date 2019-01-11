#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>

#define MAXSIZE 256
#define PORT 4178

int main()
{
	char buf[256];
	int servfd, clientfd, numbytes;
	fd_set read_set, read_set_copy;
	struct sockaddr_in serv, clientaddr;
	socklen_t sock_size = sizeof(struct sockaddr_in);
	if((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket failed.");
		exit(EXIT_FAILURE);
	}
	int optval = 1;
	if (setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)) < 0)
		exit(-1);
	bzero(&serv, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htons(INADDR_ANY);
	serv.sin_port = htons(PORT);
	if(bind(servfd, (struct sockaddr *)&serv, sizeof(serv)) < 0)
	{
		perror("Bind failed.");
		exit(EXIT_FAILURE);
	}
	if (listen(servfd, 20) < 0)
	{
		perror("Listen failed.");
		exit(EXIT_FAILURE);
	}
	FD_ZERO(&read_set);
	FD_ZERO(&read_set_copy);
	FD_SET(servfd, &read_set);
	while(1)
	{
		read_set_copy = read_set;
		if((select(FD_SETSIZE, &read_set_copy, NULL, NULL, NULL)) < 0)
		{
			perror("Select failed.");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < FD_SETSIZE; ++i)
		{
			if (FD_ISSET(i, &read_set_copy))
			{
				if (i == servfd)
				{
					if ((clientfd = accept(servfd, (struct sockaddr *)&clientaddr, &sock_size)) < 0)
						perror("Accept failed."), exit(EXIT_FAILURE);
					FD_SET(clientfd, &read_set);
				}
				else
				{
					numbytes = read(i, buf, MAXSIZE);
					buf[numbytes] = '\0';
					if (strcmp(buf, "LOGOUT\n") != 0)
					{
						printf("%s\n", buf);
						for (int n = 0; n < FD_SETSIZE; ++n)
						{
							if(FD_ISSET(n, &read_set) && n != servfd && n != i)
								write(n, buf, strlen(buf)+1);
						}
						FD_SET(i, &read_set);
					}
					else
					{
						bzero(buf, strlen(buf)+1);
						numbytes = read(i, buf, MAXSIZE);
						buf[numbytes] = '\0';
						printf("%s\n", buf);
						for (int n = 0; n < FD_SETSIZE; ++n)
						{
							if(FD_ISSET(n, &read_set) && n != servfd && n != i)
								write(n, buf, strlen(buf)+1);
						}
						close(i);
						FD_CLR(i, &read_set);
					}
					bzero(buf, strlen(buf)+1);
				}
			}
		}
	}
	close(servfd);

}
