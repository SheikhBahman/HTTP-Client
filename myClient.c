/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define MAXDATASIZE_all 10000 // max number of bytes we can store in local
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];	
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char port[INET6_ADDRSTRLEN], server[INET6_ADDRSTRLEN], address[INET6_ADDRSTRLEN];
	FILE *f = fopen("output", "wb");
	if (argc != 2) {		
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	//Bahman Start
	if (strstr(argv[1], "http://") != NULL){
		sscanf(argv[1], "http://%9999[^\n]", argv[1]);
		if (strstr(argv[1], ":") != NULL){
			sscanf(argv[1], "%99[^:]:%99[^/]/%9999[^\n]", server, port, address);
		}
		else {
			strcpy(port, "80");
			sscanf(argv[1], "%99[^/]/%9999[^\n]", server, address);
		}
	}
	else{
		fprintf(f, "%s", "INVALIDPROTOCOL"); 
		return 1;
		/*if (strstr(argv[1], ":") != NULL)
			sscanf(argv[1], "%99[^:]:%99[^/]/%99[^\n]", server, port, address);
		else {
			strcpy(port, "80");
			sscanf(argv[1], "%99[^/]/%99[^\n]", server, address);
		}*/
	}
	/*printf("server = \"%s\"\n", server);
	printf("port = \"%s\"\n", port);
	printf("address = \"%s\"\n", address);*/
	//Bahman End
	
	if ((rv = getaddrinfo(server, port, &hints, &servinfo)) != 0) {
		fprintf(f, "%s", "NOCONNECTION");
		//fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		//fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	//printf("client: connecting to %s\n", s);

	//Bahman START
	
	
	
	//char header[INET6_ADDRSTRLEN] = "";

	char *header = (char *)malloc((strlen(address) + INET6_ADDRSTRLEN));
	strcpy(header, "GET /");
	strcat(header, address);	
	strcat(header, " HTTP/1.1\nHost: ");
	strcat(header, server);
	strcat(header, ":");
	strcat(header, port);
	strcat(header, "\r\n\r\n");
	//Bahman END
	//printf("header = \"%s\"\n", header);

	if (send(sockfd, header, strlen(header), 0) == -1) {
		perror("send");
		exit(1);
	}
	free(header);
	freeaddrinfo(servinfo); // all done with this structure

	char buf_local[MAXDATASIZE_all] = "";
	bool localFull = true;

	int current = 0;
	while ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) > 0 && (localFull = (current + MAXDATASIZE < MAXDATASIZE_all))) {
		memcpy(buf_local + current, buf, numbytes);
		current += numbytes;		
		memset(buf, 0, sizeof(buf));		
	}

	if (strstr(buf_local, "200 OK") > 0){
		char *result = strstr(buf_local, "\r\n\r\n");		
		if (result != NULL){
			fwrite(buf_local + (result - buf_local) + 4, 1, current - ((result - buf_local) + 4), f);
		}

		if (!localFull){
			fwrite(buf, 1, numbytes, f);
			memset(buf, 0, sizeof buf);
			while ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) > 0) {
				fwrite(buf, 1, numbytes, f);
				memset(buf, 0, sizeof(buf));
			}
		}
	}
	else if (strstr(buf_local, "404") > 0){
		fprintf(f, "%s", "FILENOTFOUND");
	}


	close(sockfd);
	fclose(f);
	return 0;
}

