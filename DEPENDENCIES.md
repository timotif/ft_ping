# Dependencies

## Debian 11 VM Setup

```bash
sudo apt-get update
sudo apt-get install -y build-essential bc
```

**Why:**
- `build-essential` - gcc, make, libc-dev (for compilation)
- `bc` - Calculator for Makefile progress counter

**Note:** `coreutils` (ls, wc, sed, grep, clear, printf) are already installed in Debian 11

## Running

Requires root for raw ICMP sockets:
```bash
sudo ./ft_ping <destination>
```
