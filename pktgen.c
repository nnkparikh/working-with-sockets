/* CMPUT 379 Assignment 2a */
/* Student: Neel Parikh */
/* ID: 1358644 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXBUFSIZE 4096

const char * NetworkA_Hosts[] = {"192.168.128.7", "192.168.128.1"};
const char * NetworkB_Hosts[] = {"192.168.192.10", "192.168.192.6", "192.168.192.4"};
const char * NetworkC_Hosts[] = {"192.224.0.5","192.224.0.7", "192.224.10.5", "192.224.15.6"};
void usage();
/* packet gen program */
int main(int argc, char *argv[])
{

	struct sockaddr_in pktgenaddr, routeraddr;
	socklen_t router_addrlen = sizeof(routeraddr);
	int pktGenSocket;
	char buf[MAXBUFSIZE];
	enum Network{ A, B, C };
	/* check for correct command line arguments */
	if(argc != 3) usage();

	const unsigned int routerPort = atoi(argv[1]);		/* port number in order to connect to router */
	const char * packetFile_Path = argv[2];				/* packet file path */

	/* create socket from which to send */
	pktGenSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(pktGenSocket < 0)
	{
		perror("cannot create socket.");
		exit(1);
	}
	/* zero intialize the sockaddr_in struct */
	memset((char *)&pktgenaddr, 0, sizeof(pktgenaddr));
	pktgenaddr.sin_family = AF_INET;					/* address family for the socket */
	pktgenaddr.sin_port = htons(0);						/* assign any port number */
	pktgenaddr.sin_addr.s_addr = htonl(INADDR_ANY);		/* address for the socket */

	/* associate the address with the pktgen socket */
	if(bind(pktGenSocket, (struct sockaddr *) &pktgenaddr, sizeof(pktgenaddr)) < 0)
	{
		perror("bind failed.");
		exit(1);
	}

	/*the address to whom we want to send messages */
	memset((char *)&routeraddr, 0, sizeof(routeraddr));
	routeraddr.sin_family = AF_INET;					/* address family for router addr */
	routeraddr.sin_port = htons(routerPort);			/* port number to reach router */
	routeraddr.sin_addr.s_addr = htonl(INADDR_ANY);		/* any interface */

	unsigned long PacketId = 0;		/* increment packetId with each iteration */
	const char * sourceHost;				/* randomly choosen source host*/
	const char * destinationHosts[2];		/* possible destionation hosts to send to */
	const char * destinationHost;			/* randomly pick one desition host out of two options */
	unsigned short TTL;				/* Time To Live value of the packet generated */
	char payload[] = "Packet Number: ";
	enum Network net;				/* either A B or C */
	for(;;PacketId++)
	{
		/*randomly pick one out of the three networks*/
		net = rand() % 3;
		/*randomly select a TTL value between 1 and 4*/
		TTL = (rand()%4)+1;
		switch(net)
		{	
			/* From the randomly chosen network, randomly pick a host 
			   within this network. Then Randomly pick a destionation host
			   from the other two network. */
			case A:
				/* pick random host from network A to be the source host*/
				sourceHost = NetworkA_Hosts[rand()%2];
				destinationHosts[0] = NetworkB_Hosts[rand()%3];
				destinationHosts[1] = NetworkC_Hosts[rand()%4];
				break;
			case B:
				/* pick random host from network B to be the source host*/
				sourceHost = NetworkB_Hosts[rand()%3];
				destinationHosts[0] = NetworkA_Hosts[rand()%2];
				destinationHosts[1] = NetworkC_Hosts[rand()%4];
				break;
			case C:
				/* pick random host from network C to be the source host*/
				sourceHost = NetworkC_Hosts[rand()%4];
				destinationHosts[0] = NetworkA_Hosts[rand()%2];
				destinationHosts[1] = NetworkB_Hosts[rand()%3];
				break;
		}
		/* randomly select between the two possible destination hosts*/
		destinationHost = destinationHosts[rand()%2];
		/* At this point, the source host and the destionation host should have been selected */
		/* network of the source host and the desestionation host will be different */

		/*now generate packet*/
		sprintf(buf, "%lu, %s, %s, %hu, %s%lu\n",PacketId, sourceHost, destinationHost, TTL, payload,PacketId);
		printf("%s\n",buf);
		if (sendto(pktGenSocket, buf, strlen(buf)+1, 0, (struct sockaddr *)&routeraddr, router_addrlen) < 0)
			perror("error in sending packet.");
	}
	return 0;
}

void usage()
{
	perror("usage: "
			   "./a.out "
			   "<port number to connect to router> "
			   "<packet file path>\n");
	exit(1);
}