#include "ping.h"



double sum_rtt_squared = 0;

void signal_handler(int signum) {
    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    long run_time = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                    (end_time.tv_usec - start_time.tv_usec) / 1000;

    printf("\n--- Ping Statistics ---\n");
    printf("%d packets transmitted, %d received, %.1f%% packet loss, time %ldms\n",
           packets_sent, packets_received,
           ((packets_sent - packets_received) / (double)packets_sent) * 100.0,
           run_time);

    if (packets_received > 0) {
        double avg = total_rtt / packets_received;
        double variance = (sum_rtt_squared / packets_received) - (avg * avg);
        double mdev = sqrt(variance);

        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
               min_rtt, avg, max_rtt, mdev);
    }
    exit(0);
}

unsigned short calculate_checksum(void *data, int length) {
    unsigned short *ptr = data;
    unsigned int sum = 0;
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }
    if (length == 1) {
        sum += *(unsigned char *)ptr;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void send_ping(int sock, struct sockaddr *addr, socklen_t addrlen, int seq, int ipv6) {
    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    if (!ipv6) {
        struct icmphdr *icmp = (struct icmphdr *)packet;
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.id = getpid();
        icmp->un.echo.sequence = seq;

        struct timeval start;
        gettimeofday(&start, NULL);
        memcpy(packet + sizeof(struct icmphdr), &start, sizeof(start));

        icmp->checksum = calculate_checksum(packet, sizeof(struct icmphdr) + sizeof(start));
    } else {
        struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)packet;
        icmp6->icmp6_type = ICMP6_ECHO_REQUEST;
        icmp6->icmp6_code = 0;
        icmp6->icmp6_cksum = 0;
        icmp6->icmp6_id = getpid();
        icmp6->icmp6_seq = seq;

        struct timeval start;
        gettimeofday(&start, NULL);
        memcpy(packet + sizeof(struct icmp6_hdr), &start, sizeof(start));
    }

    sendto(sock, packet, sizeof(packet), 0, addr, addrlen);
    packets_sent++;
}

void receive_ping(int sock, int ipv6) {
    char buffer[1024];
    struct timeval end;
    struct sockaddr_storage recv_addr;
    socklen_t addrlen = sizeof(recv_addr);

    int bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&recv_addr, &addrlen);
    if (bytes > 0) {
        gettimeofday(&end, NULL);
        packets_received++;

        struct timeval start;
        double rtt;
        int icmp_bytes;
        int ttl;

        if (!ipv6) {
            struct iphdr *ip = (struct iphdr *)buffer;
            int iphdr_len = ip->ihl * 4;
            memcpy(&start, buffer + iphdr_len + sizeof(struct icmphdr), sizeof(start));

            struct timeval diff;
            timersub(&end, &start, &diff);
            rtt = diff.tv_sec * 1000.0 + diff.tv_usec / 1000.0;

            icmp_bytes = bytes - iphdr_len;
            ttl = ip->ttl;
        } else {
            struct ip6_hdr *ip6 = (struct ip6_hdr *)buffer;
            memcpy(&start, buffer + 40 + sizeof(struct icmp6_hdr), sizeof(start));

            struct timeval diff;
            timersub(&end, &start, &diff);
            rtt = diff.tv_sec * 1000.0 + diff.tv_usec / 1000.0;

            icmp_bytes = bytes - 40;
            ttl = ip6->ip6_hlim;
        }

        if (min_rtt < 0 || rtt < min_rtt) min_rtt = rtt;
        if (rtt > max_rtt) max_rtt = rtt;
        total_rtt += rtt;
        sum_rtt_squared += rtt * rtt;

        char ip_str[INET6_ADDRSTRLEN];
        if (!ipv6) {
            inet_ntop(AF_INET, &((struct sockaddr_in *)&recv_addr)->sin_addr, ip_str, INET6_ADDRSTRLEN);
        } else {
            inet_ntop(AF_INET6, &((struct sockaddr_in6 *)&recv_addr)->sin6_addr, ip_str, INET6_ADDRSTRLEN);
        }

        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
               icmp_bytes, ip_str, packets_received, ttl, rtt);
    }
}


int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    gettimeofday(&start_time, NULL);

    char *ip = NULL;
    int ipv6 = 0, count = -1, flood = 0, opt;

    while ((opt = getopt(argc, argv, "a:t:c:f")) != -1) {
        switch (opt) {
            case 'a':
                ip = optarg;
                break;
            case 't':
                ipv6 = atoi(optarg) == 6;
                break;
            case 'c':
                count = atoi(optarg);
                if (count <= 0) {
                    fprintf(stderr, "Invalid count. It must be a positive number.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'f':
                flood = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s -a <address> -t <4|6> [-c count] [-f]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!ip) {
        fprintf(stderr, "IP address is required. Use -a <address> to specify.\n");
        exit(EXIT_FAILURE);
    }

    int sock = socket(ipv6 ? AF_INET6 : AF_INET, SOCK_RAW, ipv6 ? IPPROTO_ICMPV6 : IPPROTO_ICMP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;
    struct sockaddr *addr;
    socklen_t addrlen;

    if (!ipv6) {
        addr4.sin_family = AF_INET;
        if (inet_pton(AF_INET, ip, &addr4.sin_addr) <= 0) {
            perror("Invalid IPv4 address");
            exit(EXIT_FAILURE);
        }
        addr = (struct sockaddr *)&addr4;
        addrlen = sizeof(addr4);
    } else {
        addr6.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, ip, &addr6.sin6_addr) <= 0) {
            perror("Invalid IPv6 address");
            exit(EXIT_FAILURE);
        }
        addr = (struct sockaddr *)&addr6;
        addrlen = sizeof(addr6);
    }

    int seq = 0;
    while (count == -1 || seq < count) {
        send_ping(sock, addr, addrlen, seq++, ipv6);

        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLIN;

        int result = poll(&pfd, 1, TIMEOUT);
        if (result == 0) {
            printf("Request timeout for icmp_seq=%d\n", seq);
        } else if (result > 0 && (pfd.revents & POLLIN)) {
            receive_ping(sock, ipv6);
        }

        if (!flood) sleep(1);
    }

    close(sock);
    signal_handler(SIGINT);
    return 0;
}
