#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 4178
#define BACKLOG 10
#define MAXSIZE 256
#define h_addr h_addr_list[0]

int get_args(char username[MAXSIZE], char hostname[MAXSIZE], int * port, int argc, char * argv[])
{
	switch(argc)
	{
	case 1:
		strcpy(username, "Default");
		strcpy(hostname, "127.0.0.1");
		*port = PORT; 
		break;
	case 2:
		strcpy(username, argv[1]);
		strcpy(hostname, "127.0.0.1");
		*port = PORT;
		break;
	case 3:
		strcpy(username, argv[1]);
		strcpy(hostname, argv[2]);
		*port = PORT;
		break;
	case 4:
		strcpy(username, argv[1]);
		strcpy(hostname, argv[2]);
		*port = atoi(argv[3]);
	}
	return 0;
}

int main(int argc, char * argv[])
{
	int servfd, port, numbytes;
	fd_set read_set, read_set_copy;
	char buf[MAXSIZE], formatted_buf[MAXSIZE], username[MAXSIZE], hostname[MAXSIZE];
	struct sockaddr_in serv;
	struct hostent * he;
	get_args(username, hostname, &port, argc, argv);
	if((he = gethostbyname(hostname)) == NULL)
	{
		perror("Invalid host.");
		return 0;
	}	
	if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Invalid socket.");
		return 0;
	}
	bzero(&serv, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr = *((struct in_addr *)he->h_addr);
	FD_ZERO(&read_set);
	FD_ZERO(&read_set_copy);
	FD_SET(servfd, &read_set);
	FD_SET(0, &read_set);
	if ((connect(servfd, (struct sockaddr*)&serv, sizeof(serv))) < 0)
	{
		perror("Connect failed.");
		close(servfd);
		exit(EXIT_FAILURE);
	}
	sprintf(buf, "%s has joined the chatroom.\n", username);
	printf("%s\n", buf);
	write(servfd, buf, strlen(buf)+1);
	do
	{
		read_set_copy = read_set;
		if (select(FD_SETSIZE, &read_set_copy, NULL, NULL, NULL) < 0)
		{
			perror("Select failed.");
			close(servfd);
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < FD_SETSIZE; ++i)
		{
			if (FD_ISSET(i, &read_set_copy))
			{
				bzero(buf, strlen(buf)+1);
				bzero(formatted_buf, strlen(formatted_buf)+1);
				numbytes = read(i, buf, MAXSIZE);
				buf[numbytes] = '\0';
				if (i == 0)
				{	
					sprintf(formatted_buf, "%s: %s", username, buf);
					printf("%s\n", formatted_buf);
					strcmp(buf, "LOGOUT\n") == 0 ? write(servfd, buf, strlen(buf) + 1) : write(servfd, formatted_buf, strlen(formatted_buf)+1);
				}
				else
					printf("%s\n", buf);
			}
		}
	} while (strcmp(buf, "LOGOUT\n") != 0);
	sprintf(buf, "%s has lefted the chatroom.\n", username);
	write(servfd, buf, strlen(buf) + 1);
	close(servfd);
	return 0;	
}


