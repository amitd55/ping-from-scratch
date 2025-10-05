#ifndef PING
#define PING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <getopt.h>
#include <math.h>
#include <netinet/ip6.h>

#define TIMEOUT 10000 // 10 seconds in milliseconds
#define PACKET_SIZE 64

// Global variables for statistics
int packets_sent = 0;
int packets_received = 0;
double min_rtt = -1, max_rtt = 0, total_rtt = 0;
struct timeval start_time;

void signal_handler(int signum);
unsigned short calculate_checksum(void *data, int length);
void send_ping(int sock, struct sockaddr *addr, socklen_t addrlen, int seq, int ipv6);
void receive_ping(int sock, int ipv6);

#endif