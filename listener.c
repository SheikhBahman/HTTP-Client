/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 100


int main(int argc,char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;

	int recv_bytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	

	getaddrinfo(NULL, MYPORT, &hints, &servinfo);

	
	p = servinfo;
	sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	bind(sockfd, p->ai_addr, p->ai_addrlen);

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	recv_bytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);

	printf("listener: packet is %d bytes long\n", recv_bytes);
	buf[recv_bytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	close(sockfd);

	return 0;
}
