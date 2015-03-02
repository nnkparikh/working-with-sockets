/* CMPUT 379 Assignment 2a */
/* Student: Neel Parikh */
/* ID: 1358644 */

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*  Program takes three command-line arguments: */
/*  <port number to listen> <routing table file path> <statistic file path> */

int main(int argc, char * argv[])
{
	/* check for invalid command line argument */
	if(argc != 4)
	{
		perror("usage: "
			   "./a.out "
			   "<port number to listen to> "
			   "<routing table file path> "
			   "<statistic file path>\n");
		exit(1);
	}
	const unsigned int UDPport = atoi(argv[1]);		/* save port number */
	const char * routingTable_Path = argv[2]; 		/* store routing table file path */
	const char * statistic_Path = argv[3]; 			/* store statistic file path */

	printf("%d %s %s\n",UDPport, routingTable_Path, statistic_Path);
	return 0;
}