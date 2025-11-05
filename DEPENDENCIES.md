# Dependencies

## Debian 11 VM Setup

```bash
apt-get update
apt-get install -y build-essential sudo libcap2-bin inetutils-ping bc
```

**Why:**
- `build-essential` - gcc, make, libc-dev (for compilation)
- `sudo` - Required for raw socket operations
- `libcap2-bin` - For setting capabilities on the binary (if needed)
- `inetutils-ping` - For comparison with system ping
- `bc` - Calculator for Makefile progress counter

**Note:** `coreutils` (ls, wc, sed, grep, clear, printf) are already installed in Debian 11
