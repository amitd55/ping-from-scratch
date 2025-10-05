# C-Ping

An implementation of the classic `ping` utility written in **C from scratch**.  
This project demonstrates working with raw sockets, ICMP/ICMPv6 packets, RTT (Round Trip Time) measurement, and statistics such as min/avg/max/mdev.  

---

## Features
- Supports **IPv4** (ICMP) and **IPv6** (ICMPv6).
- Calculates RTT for each packet.
- Displays statistics: min/avg/max/mdev.
- Supports multiple command-line flags (`-a`, `-t`, `-c`, `-f`).
- Implemented with raw sockets using `getopt`.

---

## Compilation
Use the provided `Makefile`:
```bash
make
