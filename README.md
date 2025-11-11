# ft_ping

A custom implementation of the ICMP ping utility, written in C as part of the 42 Berlin advanced curriculum.

The `ft_ping` project is part of the advanced network programming track, requiring students to implement low-level network protocols using raw sockets, handle ICMP packets, and replicate the behavior of the standard Unix `ping` utility.

## Project Overview

`ft_ping` sends ICMP Echo Request packets to a destination host and receives Echo Reply packets, measuring round-trip time (RTT) and displaying network statistics. This implementation follows the behavior of **GNU inetutils-2.0 ping**.

### Features

- ICMP Echo Request/Reply handling using raw sockets
- DNS hostname resolution
- Round-trip time (RTT) measurement with microsecond precision
- Packet loss detection and statistics
- Duplicate packet detection
- ICMP error message handling (Destination Unreachable, Time Exceeded, etc.)
- Configurable options: count, interval, TTL, timeout, verbose mode, quiet mode, flood mode
- Signal handling (graceful shutdown on SIGINT/Ctrl-C)

## Prerequisites

- Linux system (tested on Linux 6.14.0)
- GCC compiler
- Root privileges (required for raw ICMP sockets)
- Math library (`-lm`)
- `bc` for Makefile progress counter

## Installation

```bash
# Clone the repository
git clone <repository_url>
cd ft_ping

# Build the project
make

# The binary will be created as ./ft_ping
```

### Build Targets

```bash
make        # Build the project
make clean  # Remove object files
make fclean # Remove object files and binary
make re     # Rebuild from scratch
```

## Usage

```bash
# Basic usage (requires root)
sudo ./ft_ping <destination>

# Examples
sudo ./ft_ping 8.8.8.8           # Ping Google DNS
sudo ./ft_ping google.com        # Ping using hostname
sudo ./ft_ping 127.0.0.1         # Ping localhost

# With options
sudo ./ft_ping -c 5 8.8.8.8      # Send 5 packets then stop
sudo ./ft_ping -i 2 8.8.8.8      # 2-second interval between packets
sudo ./ft_ping -v 8.8.8.8        # Verbose mode (packet dumps)
sudo ./ft_ping -q -c 10 8.8.8.8  # Quiet mode (only statistics)
sudo ./ft_ping -w 3 8.8.8.8      # 3-second timeout
sudo ./ft_ping --ttl 128 8.8.8.8 # Set TTL to 128
sudo ./ft_ping -f 8.8.8.8        # Flood mode (requires root)
```

### Command-line Options

| Flag | Description |
|------|-------------|
| `-c <count>` | Stop after sending `count` packets |
| `-i <interval>` | Wait `interval` seconds between packets (default: 1) |
| `-w <timeout>` | Time to wait for response in seconds |
| `--ttl <ttl>` | Set Time To Live |
| `-v` | Verbose output with packet dumps |
| `-q` | Quiet mode (no per-packet output) |
| `-f` | Flood mode - send packets as fast as possible |
| `-l <preload>` | Send preload packets as fast as possible before going into normal mode |
| `-V` | Display version information |
| `-?`, `--help` | Display help message |
| `--usage` | Display brief usage information |

## Architecture

The codebase follows a **layered architecture** with clear separation of concerns:

```
ft_ping.c          - Application entry point, lifecycle, signal handling
network.c          - Network layer: sockets, DNS, packet I/O
ping.c             - Core ping logic: event loop, stats, timing
icmp_packet.c      - ICMP protocol: packet construction, parsing, checksum
ip_header.c        - IP protocol: header validation and utilities
parse.c            - Command-line argument parsing
output_format.c    - User-facing output formatting
output_debug.c     - Diagnostic output (verbose mode)
time_utils.c       - Timing utilities for RTT calculation
bitmap.c           - Duplicate packet detection using bitmasks
```

### Key Implementation Details

- **Raw ICMP sockets**: Requires root privileges for packet construction
- **Kernel timestamps**: Uses `SO_TIMESTAMP` socket option for accurate RTT measurement
- **DNS resolution**: IPv4-only via `getaddrinfo()`, uses first result
- **Packet filtering**: Validates ICMP ID (matches PID)
- **Statistics**: Real-time min/avg/max/stddev calculation using Welford's algorithm
- **Duplicate detection**: Efficient bitmap tracking of received sequences
- **Exit on error pattern**: Initialization functions exit directly on fatal errors

## Output Format

### Successful Ping
```
PING 8.8.8.8 (8.8.8.8): 56 data bytes
64 bytes from 8.8.8.8: icmp_seq=0 ttl=117 time=14.2 ms
64 bytes from 8.8.8.8: icmp_seq=1 ttl=117 time=13.8 ms
64 bytes from 8.8.8.8: icmp_seq=2 ttl=117 time=14.1 ms
^C
--- 8.8.8.8 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 13.800/14.033/14.200/0.173 ms
```

### ICMP Errors
```
From 192.168.1.1: Destination Host Unreachable
From 192.168.1.1: Time to live exceeded
```

Quick sanity checks:
```bash
sudo ./ft_ping 127.0.0.1    # Should succeed with <1ms RTT
sudo ./ft_ping 8.8.8.8      # Should succeed with 10-100ms RTT
sudo ./ft_ping 192.0.2.1    # Should timeout (non-routable TEST-NET-1)
```

## Reference Implementation

The project mimics the behavior of **GNU inetutils-2.0 ping**.

Key behavioral traits:
- Sequence numbers start at 0 (not 1)
- No per-packet timeout by default (waits indefinitely)
- Statistics format: `round-trip min/avg/max/stddev = ...`
- Exit code 0 on SIGINT, 1 on 100% packet loss with `-c` flag

## Technical Documentation

- **DEPENDENCIES.md**: System dependencies and setup instructions. To be evaluated at 42, the project is run in a virtual machine running Debian >= 7.0

## Compilation Flags

The project uses strict compilation standards per 42 school requirements:
```
-Wall -Wextra -Werror
```
All warnings are treated as errors.

## Project Structure

```
ft_ping/
├── src/                 # Source files (.c)
├── inc/                 # Header files (.h)
├── obj/                 # Object files (generated)
├── sandbox/             # Testing and experimentation
├── Makefile             # Build configuration
├── README.md            # This file
├── DEPENDENCIES.md      # System dependencies
└── en.subject.ft_ping.pdf  # 42 project subject
```

## Standards and RFCs

- **RFC 792**: Internet Control Message Protocol (ICMP)
- **RFC 1071**: Computing the Internet Checksum
- **RFC 1122**: Requirements for Internet Hosts

## Author

**tfregni** - 42 Berlin

## License

This project is part of the 42 School curriculum and follows 42's academic policies.

## About 42 School

[42](https://[https://42berlin.de/]) is a global network of tuition-free, peer-to-peer coding schools with a unique pedagogy that emphasizes project-based learning without teachers or traditional lectures. Students (called "cadets") progress through increasingly complex projects at their own pace, learning through hands-on practice and collaboration with peers.

Key characteristics of 42:
- **Project-based learning**: All learning happens through practical coding projects
- **Peer evaluation**: Students review and grade each other's work
- **No teachers**: Learning is self-directed with support from peers
- **24/7 access**: Campuses are open around the clock
- **Real-world skills**: Projects simulate professional software development challenges

## Acknowledgments

- 42 Berlin for the project subject and peer-learning environment
- GNU inetutils project for the reference implementation
- The peer evaluation system at 42 that helps refine and improve projects
