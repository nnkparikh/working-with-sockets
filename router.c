/* CMPUT 379 Assignment 2a */
/* Student: Neel Parikh */
/* ID: 1358644 */
/* router program */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAXBUFSIZE 4096
#define LINESIZE 80
#define NUMOFHOSTS 2
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
	unsigned long directDelPkt_count;			/* count of packets directly delivered (to network A)*/
	unsigned long BPkt_count;					/* count of forwarded packets to network B */
	unsigned long CPkt_count;					/* count of forwarded packets to network C */ 
	unsigned long unroutedPkt_count;			/* count of dropped packets */
	unsigned long expiredPkt_count;				/* count of expired packets */
} stats = {};

enum Route{ A, B, C, FAIL};

void usage();
void updateFile();
void signal_handler(int);
int getPktInfo(char buf[]);							/* retreive the info from packets store into pktInfo struct*/
enum Route findRoutingPath(struct sockaddr_in);		/* to determine which network to forward the packet*/

FILE *fp_route;	/* global file pointer to reference the routing table file path */
FILE *fp_stats;	/* global file pointer to reference the statistic file path */
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
	enum Route pktRoute;

	/* check for correct command line argument */
	if(argc != 4) usage();

	const unsigned int UDPport = atoi(argv[1]);		/* save port number */
	const char * routingTable_Path = argv[2]; 		/* store routing table file path */
	const char * statistic_Path = argv[3]; 			/* store statistic file path */

	/* report error if routing table file cannot be opened */
	if((fp_route = fopen(routingTable_Path,"r")) == NULL)
	{
		perror("routing table path cannot be opened.");
		exit(1);
	}
	/* report error if statistic file cannot be opened */
	if((fp_stats = fopen(statistic_Path,"w")) == NULL)
	{
		perror("statistic file cannot be opened.");
		exit(1);
	}

	/*declare custom signal handler*/
    if(signal(SIGINT, signal_handler) == SIG_ERR)
    	perror("can't catch SIGSEGV\n");

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
	unsigned long packetCounter = 0;	
	/* continuously receive data from port */
	for(;;)
	{
		recv_len = recvfrom(routerSocket, buf, MAXBUFSIZE, 0,
							(struct sockaddr *) &remoteaddr, &remote_addrlen);
		if(recv_len > 0)
		{
			printf("\nreceived message: %s\n", buf);
			unsigned int readingStatus = getPktInfo(buf);

			/*received packet*/
			packetCounter++;
			/* The packet was processed and the TTL became zero */
			if(readingStatus == 0)
			{
				/* TLL is zero */
				printf("\texpired packet.\n");
				stats.expiredPkt_count++;	/* increment expired packets count */
			}
			/* The packet was processed and it was a valid packet */
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
				printf("\tTTL: %hu\n\n", pktInfo.timetolive);
				/* Determine where this packet should be forwarded */
				pktRoute = findRoutingPath(pktInfo.destination);
				switch(pktRoute)
				{
					/* forward packet to network A*/
					case A:
						printf("\tThis packet will be directly delivered.\n");
						stats.directDelPkt_count++;
						break;
					/* forward packet to network B*/
					case B:
						printf("\tThis packet will be forwarded to routerB.\n");
						stats.BPkt_count++;
						break;
					/* forward packet to network C*/
					case C:
						printf("\tThis packet will be forwarded to routerC.\n");
						stats.CPkt_count++;
						break;
					/* packet cannot be routed */
					case FAIL:
						printf("\tThis packet cannot be routed.\n");
						stats.unroutedPkt_count++;
						break;
				}
			}
			/* The packet was processed and it did not have a valid destination address*/
			else
			{
				printf("\tinvalid destination address packet.\n");
				stats.unroutedPkt_count++;	/* increment unrouted packet count */
			}
			if((packetCounter%20) == 0)
				updateFile();
		}
		/* could not receive packet */
		else if(recv_len < 0)
		{
			perror("error is receiving packet.");
            /* do not exit */
		}
	} 
	/* never exits */
	return 0;
}

/* handle the the signal*/
void signal_handler(int signo){ 
	if (signo == SIGINT)
	{
		printf("\n\nupdating statistics file before exiting..\n"); 
    	updateFile();
    	fclose(fp_stats); /* close the statistics file*/
    	fclose(fp_route); /* close the routing file*/
    	exit(1);
	}
	perror("could not handle this signal.\n");
}

void updateFile()
{
	rewind(fp_stats); /*reset the file pointer to the beginning of the file */
	fprintf(fp_stats,"\nexpired packets: %lu\n\n", stats.expiredPkt_count);
	fprintf(fp_stats,"unroutable packets: %lu\n\n", stats.unroutedPkt_count);
	fprintf(fp_stats,"delivered direct: %lu\n\n", stats.directDelPkt_count);
	fprintf(fp_stats,"router B: %lu\n\n", stats.BPkt_count);
	fprintf(fp_stats,"router C: %lu\n\n", stats.CPkt_count);
}

enum Route findRoutingPath(struct sockaddr_in packetDestination)
{
	char line[LINESIZE];	/* store each line read in file */

	/*for each line read*/
	char NetIP[INET_ADDRSTRLEN];	/* store the network address */
	unsigned short prefix;			/* store the network prefix*/
	char router[8];					/* store the hop value*/


	char DestIPMasked[INET_ADDRSTRLEN];		/* store the destination IP string in dot notation after masked*/

	/*also need certain values to store the the masked IP in network byte order*/
	struct in_addr destinAddr = packetDestination.sin_addr;
	unsigned long destinIP = ntohl(destinAddr.s_addr);
	unsigned long destinIP_masked;

	enum Route takeRoute;

	rewind(fp_route); /*reset the file pointer to the beginning of the file */
	while(fgets(line,LINESIZE,fp_route))
	{
		size_t ln = strlen(line)-1;
		if((line[0] != '\n') && (line[0] != '\r'))
		{
			line[ln] = '\0';
			sscanf(line,"%s %hu %s",NetIP,&prefix,router);

			if(strcmp(router,"RouterB") == 0) takeRoute = B;
			else if(strcmp(router,"RouterC") == 0) takeRoute = C;
			else takeRoute = A;

			/*extract <prefix> number of bits of the destination IP*/
			destinIP_masked = destinIP & (0xFFFFFFFF << (32-prefix));
			/*convert the masked destinIP to network order byte*/
			destinAddr.s_addr = htonl(destinIP_masked);
			/* get the string dot notation representation of this IP */
			inet_ntop(AF_INET, &(destinAddr), DestIPMasked, INET_ADDRSTRLEN);

			if(strcmp(NetIP,DestIPMasked) == 0)
				return takeRoute;

		}
	}
	
	return FAIL;
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
