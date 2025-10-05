
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
````

Clean build files:

```bash
make clean
```

---

## Usage

The program supports the following flags:

* **`-a <address>`**
  Destination IP address to ping.
  *Required.*

* **`-t <type>`**
  Protocol type:

  * `4` → IPv4 (ICMP)
  * `6` → IPv6 (ICMPv6)
    *Required.*

* **`-c <count>`**
  Number of pings to send.
  *Optional.*
  Example: `-c 4` → send 4 pings.

* **`-f` (flood)**
  Send ping packets continuously without delay.
  *Optional.*

---

## Examples

```bash
# Ping Google DNS (IPv4), 4 packets
sudo ./ping -a 8.8.8.8 -t 4 -c 4

# Ping Google DNS (IPv6), flood mode
sudo ./ping -a 2001:4860:4860::8888 -t 6 -f
```

---

## Sample Output

```
64 bytes from 8.8.8.8: icmp_seq=1 ttl=116 time=12.802 ms
64 bytes from 8.8.8.8: icmp_seq=2 ttl=116 time=16.727 ms
64 bytes from 8.8.8.8: icmp_seq=3 ttl=116 time=11.759 ms
64 bytes from 8.8.8.8: icmp_seq=4 ttl=116 time=10.464 ms

--- Ping Statistics ---
4 packets transmitted, 4 received, 0.0% packet loss, time 4073ms
rtt min/avg/max/mdev = 10.464/12.938/16.727/2.339 ms
```

---

## Notes

* Root privileges are required to create raw sockets (run with `sudo`).
* IPv6 requires a network environment that supports IPv6 traffic.
* This project was built for educational purposes to better understand networking internals.


