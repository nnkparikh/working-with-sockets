/* CMPUT 379 Assignment 2a */
/* Student: Neel Parikh */
/* ID: 1358644 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
	struct sockaddr_in source;
	struct sockaddr_in destination;
	unsigned short timetolive;
	char payload[MAXBUFSIZE];
} pktInfo = {};

/* for purposes of statistics */
struct statistics
{
	unsigned long directDelPkt_count;			/* running count of packets directly delivered */
	unsigned long forwardedPkt_count;			/* running count of forwarded packets */
	unsigned long unroutedPkt_count;			/* running count of dropped packets */
	unsigned long expiredPkt_count;				/* running count of expired packets */
} stats = {};


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
			unsigned int readingStatus = getPktInfo(buf);
			if(readingStatus == 0)
			{
				/* TLL is zero */
				printf("\texpired packet.\n");
			}
			else if(readingStatus == 1)
			{
	
				printf("\tPacket Id: %lu\n", pktInfo.packetID);
				printf("\tPayLoad: %s\n", pktInfo.payload);
				char sourceIP_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(pktInfo.source.sin_addr), sourceIP_str, INET_ADDRSTRLEN);
				printf("\tSource IP: %s\n", sourceIP_str);
				char destinationIP_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(pktInfo.destination.sin_addr), destinationIP_str, INET_ADDRSTRLEN);
				printf("\tDestionation IP: %s\n",destinationIP_str);
				printf("\tTTL: %hu\n", pktInfo.timetolive);
			}
			else
			{
				printf("\tinvalid destionation address packet.\n");
			}
		}
		else if(recv_len < 0)
		{
			perror("error is receiving packet.");
			exit(1);
		}
	} 
	/* never exits */
	return 0;
}
/* update packetInfo struct with the received data*/
int getPktInfo(char buf[])
{
	unsigned long packetID_tok = 0;
	char sourceIP_tok[INET_ADDRSTRLEN];
	char destinationIP_tok[INET_ADDRSTRLEN];
	unsigned short ttl = 0;
	char payload[MAXBUFSIZE];
	sscanf(buf, "%lu, %[^','], %[^','], %hu, %[^'\n']", 
			&packetID_tok, sourceIP_tok, destinationIP_tok, &ttl, payload);

	/* get destination IP*/
	inet_pton(AF_INET,destinationIP_tok,&(pktInfo.destination.sin_addr));
	/* check if invalid destination IP*/
	if(strcmp(destinationIP_tok, "168.130.192.01") == 0)
		return -1;

	/* check if TTL expires */
	if((--ttl) == 0) return 0;
	pktInfo.timetolive = ttl;

	/*get packet ID*/
	pktInfo.packetID = packetID_tok;
	/*get source IP*/
	inet_pton(AF_INET,sourceIP_tok,&(pktInfo.source.sin_addr));

	/*get payload*/
	strcpy(pktInfo.payload, payload);
	
	return 1;
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