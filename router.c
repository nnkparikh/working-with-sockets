/* CMPUT 379 Assignment 2a */
/* Student: Neel Parikh */
/* ID: 1358644 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage()
{
	perror("usage: "
			   "./a.out "
			   "<port number to listen to> "
			   "<routing table file path> "
			   "<statistic file path>\n");
	exit(1);
}

/*  Program takes three command-line arguments: */
/*  <port number to listen> <routing table file path> <statistic file path> */

int main(int argc, char * argv[])
{
	/* check for invalid command line argument */
	if(argc != 4) usage();

	const unsigned short UDPport = atoi(argv[1]);	/* save port number */
	const char * routingTable_Path = argv[2]; 		/* store routing table file path */
	const char * statistic_Path = argv[3]; 			/* store statistic file path */

	struct sockaddr_in sockaddr;
	/* create socket from which to read */
	int UDPsocket = socket(AF_INET, SOCK_DGRAM, 0);
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
	if(bind(UDPsocket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
	{
		perror("bind failed.");
		exit(1);
	}
	printf("Port number = %d\n", ntohs(sockaddr.sin_port));
	printf("%d %s %s\n",UDPport, routingTable_Path, statistic_Path);
	return 0;
}