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
#define IPSIZE 16

/* for storing received packet information */
struct packetInfo
{
	unsigned long packetID;
	unsigned long sourceIP;
	unsigned long destinationIP;
	unsigned short timetolive;
	unsigned char payload[MAXBUFSIZE];
} pktInfo;

/* for purposes of statistics */
struct statistics
{
	unsigned long directDelPkt_count;			/* running count of packets directly delivered */
	unsigned long forwardedPkt_count;			/* running count of forwarded packets */
	unsigned long unroutedPkt_count;			/* running count of dropped packets */
	unsigned long expiredPkt_count;				/* running count of expired packets */
} stats;


void usage();
int getPktInfo(char buf[]);
/*  Program takes three command-line arguments: */
/*  <port number to listen> <routing table file path> <statistic file path> */
/* the router program */
int main(int argc, char * argv[])
{

	struct sockaddr_in routeraddr, remoteaddr;
	socklen_t remote_addrlen = sizeof(remoteaddr);
	int recv_len;	/* to store return value from recvfrom()*/
	int routerSocket;	/* to store socket descriptor*/
	char buf[MAXBUFSIZE];

	/* check for correct command line argument */
	if(argc != 4) usage();

	const unsigned int UDPport = atoi(argv[1]);		/* save port number */
	const char * routingTable_Path = argv[2]; 		/* store routing table file path */
	const char * statistic_Path = argv[3]; 			/* store statistic file path */

	
	/* create socket from which to read */
	routerSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(routerSocket < 0)
	{
		perror("cannot create socket.");
		exit(1);
	}
	/* zero intialize the sockaddr_in struct */
	memset((char *)&routeraddr, 0, sizeof(routeraddr));
	routeraddr.sin_family = AF_INET;					/* address family for the socket */
	routeraddr.sin_port = htons(UDPport);				/* assign any port number */
	routeraddr.sin_addr.s_addr = htonl(INADDR_ANY);		/* any interface */

	/* associate the address with the socket */
	if(bind(routerSocket, (struct sockaddr *) &routeraddr, sizeof(routeraddr)) < 0)
	{
		perror("bind failed.");
		exit(1);
	}
	
	printf("Port number = %d\n", ntohs(routeraddr.sin_port));	
	/* continuously receive data from port */
	for(;;)
	{
		recv_len = recvfrom(routerSocket, buf, MAXBUFSIZE, 0,
							(struct sockaddr *) &remoteaddr, &remote_addrlen);
		if(recv_len > 0)
		{
			printf("received message: %s\n", buf);
			getPktInfo(buf);
		}
	} 
	/* never exits */
	return 0;
}
/* update packetInfo struct with the received data*/
int getPktInfo(char buf[])
{
	char delimiter[2] = ",";
	char * token;
	/*get packet ID*/
	token = strtok(buf,delimiter);
	pktInfo.packetID = (unsigned long) atol(token);
	/*get source IP*/
	token = strtok(NULL, delimiter);
	
	/* get destination IP*/
	token = strtok(NULL, delimiter);
	

	/* get TTL */
	token = strtok(NULL, delimiter);
	unsigned short ttl = (unsigned short) atoi(token);
	if((--ttl) == 0) return -1;
	pktInfo.timetolive = ttl;

	/*get payload*/
	

	
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