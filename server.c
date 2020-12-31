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

int PORT = 10012;
char *PATH;
extern int errno;

int listenerfd, clients[1000];




char *readFile(char *filename) {
	char *buffer = NULL;
	
	int fileSize, readSize;
	FILE *file = fopen(filename, "r");
	
	if (file) {
		
		//seek pana la ultimul caracter din fisier
		fseek(file, 0, SEEK_END);
		
		//marimea este pozitia curenta
		fileSize = ftell(file);
		
		//inapoi la inceputul fisierului
		rewind(file);
		
		
		//buffer care sa aiba loc pentru dimensiunea fisierului
		buffer = (char*)malloc(sizeof(char) * (fileSize + 1));
		
		//citim tot fisierul
		readSize = fread(buffer, sizeof(char), fileSize, file);
		
		buffer[fileSize] = '\0';
		
		
		//eroare
		if (fileSize != readSize) {
			free(buffer);
			buffer = NULL;
		}
		
		fclose(file);
	}
	
	return buffer;
}




int handleRequest(int n) {
	char request[10000], *requestHeader[5], *extension;
	int requestLength; /* lungimea request-ului in bytes pentru folosirea functiei recv() */
	int file;
	
	
	char path[1000], *data, buffer[128];
	int dataLength; 
	
	
	memset((void*)request, (int)'\0', 10000);
	requestLength = recv(clients[n], request, 10000, 0);
	
	
	if (requestLength < 0) {
		perror("[SERVER] recv() error\n");
		return errno;
	}
	else {
		//printf("%s", request);
		//fwrite(request, 1, sizeof(request), stdout);
		
		//dummy response pentru testare
		char *response1 = "HTTP/1.0 404 Not Found";
		write(clients[n], response1, strlen(response1));
		
		
		//ne intereseaza ce fisier cere clientul in header-ul requestului, il trunchiem
		
		//prima linie
		requestHeader[0] = strtok(request, "\n");
		printf("requestHeader[0] = %s\n\n", requestHeader[0]);
		
		//operatia (GET, POST, etc)
		requestHeader[1] = strtok(requestHeader[0], " \t");
		printf("requestHeader[1] = %s\n\n", requestHeader[1]);
		
		//ne intereseaza doar GET, vrem fisierul cerut
		
		if (strncmp(requestHeader[1], "GET", 3) == 0) {
		
			//fisierul cerut
			requestHeader[2] = strtok(NULL, " \t\n");
			printf("requestHeader[2] = %s\n\n", requestHeader[2]);
			
			//versiunea HTTP
			requestHeader[3] = strtok(NULL, " \t\n");
			printf("requestHeader[3] = %s\n\n", requestHeader[3]);
			
			
			//formam path-ul catre fisierul cerut, folosindu-ne de root PATH-ul pe care l-am initializat cu current working directory
			strcpy(path, PATH);
			//printf("%s\n", path);
			strcat(path, requestHeader[2]);
			printf("%s\n", path);
			
			
			//daca nu este mentionat un fisier in request, ii dam index.html
			if (strncmp(requestHeader[2], "/\0", 2) == 0) {
				strcat(path, "index.html");
				printf("%s\n", path);
			}
			
			//trimitem doar html si txt, extragem extensia fisierului
			char pathCopy[1000];
			strcpy(pathCopy, path);
			extension = strtok(pathCopy, ".");
			extension = strtok(NULL, "\0");
			printf("%s\n", extension);
			
			if (strncmp(extension, "html\0", 5) == 0 || strncmp(extension, "txt\0", 4) == 0) {
				
				//printf("%s\n", path);
				//file = open(path, O_RDONLY);
				//printf("%d\n", file);
				
				if (access(path, F_OK) != 0) {
					char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 13\n\n404 Not Found";
					write(clients[n], response, strlen(response));
				}
				
				else {
					
					//construim header-ul response-ului
					char response[100];
					strcpy(response, "HTTP1.1 200 OK\nContent-Type: text/");
					
					//content-type este text/plain pentru .txt si text/html pentru .html
					if (strncmp(extension, "html\0", 5) == 0) 
						strcat(response, "html\n");
					else 
						strcat(response, "plain\n");
					
					strcat(response, "Content-Length: ");
					
					
					//citim fisierul
					data = readFile(path);
					
					//completam header-ul cu lungimea data-ului pe care il trimitem
					char contentLength[25];
					sprintf(contentLength, "%lu\n\n", strlen(data));
					strcat(response, contentLength);
					
					
					//trimitem header-ul si continutul fisierului
					write(clients[n], response, strlen(response));
					write(clients[n], data, strlen(data));
					
					printf("%s\n", response);
					
				}
			
			}
			
			else {
				char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 13\n\n404 Not Found";
				write(clients[n], response, strlen(response));
			}
		}
		
		else {
			char *response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 11\n\nBad Request";
			write(clients[n], response, strlen(response));
		}
		
		return 0;
	}

}


int main(int argc, char **argv) {
	
	// structuri pentru client si server
	struct sockaddr_in client;
	socklen_t length;
	struct sockaddr_in server;
	
	int clientNo = 0;
	PATH = getenv("PWD"); //setam path-ul pentru fisiere la directorul curent


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
		clients[clientNo] = accept(listenerfd, (struct sockaddr*)&client, &length);
		
		if (clients[clientNo] == -1) {
			perror("[SERVER] accept error");
			return errno;
		}
		else {
			
			pid = fork();
			// facem handle la client in procesul copil
			if (pid == 0) {
				
				close(listenerfd);
				
				
				int good = handleRequest(clientNo);
				clientNo++;
					
				exit(0);
			}
		}
	}
	
	return 0;
}



















