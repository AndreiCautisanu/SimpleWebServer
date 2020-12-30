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

int PORT = 10000;
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
		
		int pid;
		length = sizeof(client);
		clients[clientNo] = accept(listenerfd, (struct sockaddr*)&client, &length);
		
		if (clients[clientNo] == -1) {
			perror("[SERVER] accept error");
			return errno;
		}
		else {
			
			// facem handle la client in procesul copil
			if (pid = fork() == 0) {
				
				// preluam request-ul si il afisam pe ecran
				char request[10000];
				int requestLength; // lungimea request-ului in bytes pentru folosirea functiei recv()
				memset((void*)request, (int)'\0', 10000);
				requestLength = recv(clients[clientNo], request, 10000, 0);
				printf("%s", request);
			}
		}
	}
	
	return 0;
}



















