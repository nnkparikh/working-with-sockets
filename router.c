/* CMPUT 379 Assignment 2a */
/* Student: Neel Parikh */
/* ID: 1358644 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXSIZE 4096

void usage();	/*to indicate usage of program*/

/*  Program takes three command-line arguments: */
/*  <port number to listen> <routing table file path> <statistic file path> */
int main(int argc, char * argv[])
{

	struct sockaddr_in sockaddr, remoteaddr;
	socklen_t remote_addrlen = sizeof(remoteaddr);
	int recv_len;
	int UDPsocket;
	unsigned char buf[MAXSIZE];

	/* check for invalid command line argument */
	if(argc != 4) usage();

	const unsigned short UDPport = atoi(argv[1]);	/* save port number */
	const char * routingTable_Path = argv[2]; 		/* store routing table file path */
	const char * statistic_Path = argv[3]; 			/* store statistic file path */

	
	/* create socket from which to read */
	UDPsocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(UDPsocket < 0)
	{
		perror("cannot create socket.");
		exit(1);
	}
	/* zero intialize the socketaddre_in struct */
	memset((char *)&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;					/* address family for the socket */
	sockaddr.sin_port = htons(UDPport);				/* assign given port number */
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);	/* address for the socket */

	/* associate the address with the socket */
	if(bind(UDPsocket, &sockaddr, sizeof(sockaddr)) < 0)
	{
		perror("bind failed.");
		exit(1);
	}
	
	printf("Port number = %d\n", ntohs(sockaddr.sin_port));

	/* continuously receive data from port */
	for(;;)
	{
		recv_len = recvfrom(UDPsocket, buf, MAXSIZE, 0, &remoteaddr, &remote_addrlen);
		if(recv_len > 0)
		{
			printf("received message: \"%s\"\n", buf);
		}
	} 
	/* never exits */
	return 0;
}

void usage()
{
	perror("usage: "
			   "./a.out "
			   "<port number to listen to> "
			   "<routing table file path> "
			   "<statistic file path>\n");
	exit(1);
}