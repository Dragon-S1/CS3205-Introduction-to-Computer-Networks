#include "bits/stdc++.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <signal.h>

using namespace std;

// Define the Packet Constants

#define PING_PKT_SIZE 64							// Ping packet size
#define PORT_NO 0									// Ping port number
#define RECV_TIMEOUT 2						// Gives the timeout delay for receiving packetsin seconds

// Ping packet structure
struct ping_pkt{
	struct icmphdr hdr;
	char msg[PING_PKT_SIZE-sizeof(struct icmphdr)];
};

// Calculating the Check Sum
unsigned short checksum(void *b, int len){
	unsigned short *buf = (unsigned short *)b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

// make a ping request
void send_ping(int sockfd, struct sockaddr_in *ping_addr, char *ping_ip){
	int ttl_val=64, flag=1;
	socklen_t addr_len;
	
	struct ping_pkt pkt;
	struct sockaddr_in r_addr;

	// set timeout for receiving packet to 1s
	struct timeval tv_out;
	tv_out.tv_sec = RECV_TIMEOUT;
	tv_out.tv_usec = 0;
	
	// set socket options at ip to TTL and value to ttl_val
	if (setsockopt(sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0){
		printf("\nSetting socket options to TTL failed!\n");
		return;
	}

	// setting timeout of recv setting
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

	// send icmp packet in an infinite loop

	// flag is whether packet was sent or not
	flag=1;

	//filling packet
	bzero(&pkt, sizeof(pkt));
	
	pkt.hdr.type = ICMP_ECHO;
	
	for (int i = 0; i < sizeof(pkt.msg); i++ )
		if(i == sizeof(pkt.msg)-1)
			pkt.msg[i] = 0;
		else
			pkt.msg[i] = i+'A';
	
	pkt.hdr.checksum = checksum(&pkt, sizeof(pkt));

	//send packet
	auto start = std::chrono::system_clock::now();
  std::time_t game_start = std::chrono::system_clock::to_time_t(start);
	if(sendto(sockfd, &pkt, sizeof(pkt), 0,(struct sockaddr*) ping_addr, sizeof(*ping_addr)) <= 0){
		printf("\nPacket Sending Failed!\n");
		flag=0;
	}

	//receive packet
	addr_len=sizeof(r_addr);
	if(recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0 && flag){
		printf("Packet receive failed!\n");
	}
	else{
		auto end = std::chrono::system_clock::now();
		std::time_t game_end = std::chrono::system_clock::to_time_t(end);
		std::chrono::duration<double> elapsed_seconds = end-start;
		long double rtt = elapsed_seconds.count() * 1000; 
		
		// if packet was not sent, don't receive
		if(flag){
			if(!(pkt.hdr.type == 69 && pkt.hdr.code == 0)){
				if(pkt.hdr.type == 3 || pkt.hdr.type == 11)
					printf("\nRequest timed out or host unreacheable\n");
				else
					printf("Error: Packet received with ICMPtype %d & code %d\n", pkt.hdr.type, pkt.hdr.code);
			}
			else{
				cout << "Reply from " << ping_ip << " RTT = " << rtt << "ms\n" <<endl;
			}
		}
	}						
}

// Driver Code
int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in addr_con;

	if(argc!=2){
		printf("Invalid Format");
		printf("\nCorrect Format: %s <ip>\n", argv[0]);
		return 0;
	}

	if(gethostbyname(argv[1]) == NULL){
		printf("Bad hostname\n");
		return 0;
	}

	// assign IP, PORT
	addr_con.sin_family = AF_INET;
	addr_con.sin_port = htons (PORT_NO);
	addr_con.sin_addr.s_addr = inet_addr(argv[1]);

	printf("\nConnecting to '%s'\n", argv[1]);

	// create socket
	if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))<0){
		printf("\nSocket creation failed\n");
		return 0;
	}

	//send ping
	send_ping(sockfd, &addr_con, argv[1]);

	return 0;
}
