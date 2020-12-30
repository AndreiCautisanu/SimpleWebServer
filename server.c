#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

int PORT = 10006;
extern int errno;

int listenerfd, clients[1000];


int main(int argc, char **argv) {
	
	// structuri pentru client si server
	struct sockaddr_in client;
	socklen_t length;
	struct sockaddr_in server;
	
	int clientNo = 0;


	if ((listenerfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("[SERVER] socket creation error");
		return errno;
	}
	
	
	// pregatim si umplem structura folosita de server conform modelului din laborator
	bzero(&server, sizeof(server));
	bzero(&client, sizeof(client));
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);
	
	
	// atasam socketul
	if (bind(listenerfd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1) {
		perror("[SERVER] socket bind error");
		return errno;
	}
	
	// ascultam pentru conexiuni
	if (listen(listenerfd, 1000) == -1) {
		perror("[SERVER] listen error");
		return errno;
	}
	
	printf("Server started on port %d, waiting for connections\n", PORT);
	
	
	while(1) {
		
		int pid, clientfd;
		length = sizeof(client);
		clientfd = accept(listenerfd, (struct sockaddr*)&client, &length);
		
		if (clients[clientNo] == -1) {
			perror("[SERVER] accept error");
			return errno;
		}
		else {
			
			pid = fork();
			// facem handle la client in procesul copil
			if (pid == 0) {
				
				close(listenerfd);
				// preluam request-ul si il afisam pe ecran
				
				
				char request[10000], *requestHeader[10];
				int requestLength; // lungimea request-ului in bytes pentru folosirea functiei recv()
				memset((void*)request, (int)'\0', 10000);
				requestLength = recv(clientfd, request, 10000, 0);
				
				
				if (requestLength < 0) {
					perror("[SERVER] recv() error\n");
					return errno;
				}
				else {
					//printf("%s", request);
					//fwrite(request, 1, sizeof(request), stdout);
					printf("%d\n", pid);
					
					char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
					write(clientfd, response, strlen(response));
					
					
					//ne intereseaza ce fisier cere clientul in header-ul requestului, il trunchiem
					
					//prima linie
					requestHeader[0] = strtok(request, "\n");
					printf("requestHeader[0] = %s\n\n", requestHeader[0]);
					
					//operatia (GET, POST, etc)
					requestHeader[1] = strtok(requestHeader[0], " \t");
					printf("requestHeader[1] = %s\n\n", requestHeader[1]);
					
					//fisierul cerut
					requestHeader[2] = strtok(NULL, " \t\n");
					printf("requestHeader[2] = %s\n\n", requestHeader[2]);
					
					//versiunea HTTP
					requestHeader[3] = strtok(NULL, " \t\n");
					printf("requestHeader[3] = %s\n\n", requestHeader[3]);
					
					
					exit(0);
				}
			}
		}
	}
	
	return 0;
}



















