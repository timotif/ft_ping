# ft_ping Testing Guide

A comprehensive guide for testing various ping scenarios and error conditions.

**Important Note:** This guide's expected outputs are based on **inetutils-2.0** ping behavior. Key differences from other implementations:
- **No per-packet timeout by default** - ping waits indefinitely for replies unless interrupted
- Sequence numbers start at 0 (not 1)
- Statistics format: `round-trip min/avg/max/stddev = ...`
- Exit code 0 when interrupted with Ctrl-C (even with packet loss)
- Exit code 1 when using `-c` option and 100% packet loss occurs

---

## Table of Contents
1. [Basic Success Tests](#basic-success-tests)
2. [Timeout Tests](#timeout-tests)
3. [ICMP Error Tests](#icmp-error-tests)
4. [Network Simulation Tests](#network-simulation-tests)
5. [Edge Case Tests](#edge-case-tests)
6. [Comparison Testing](#comparison-testing)

---

## Basic Success Tests

### Test 1: Localhost
```bash
sudo ./ft_ping 127.0.0.1
```
**Expected Output (inetutils-2.0):**
```
PING 127.0.0.1 (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.038 ms
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.041 ms
64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.043 ms
^C
--- 127.0.0.1 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.038/0.041/0.043/0.000 ms
```
**Expected:** Very fast replies (< 0.1ms), 0% packet loss, TTL=64, runs indefinitely until Ctrl-C, exit code 0

### Test 2: Public DNS Servers
```bash
sudo ./ft_ping 8.8.8.8          # Google DNS
sudo ./ft_ping 1.1.1.1          # Cloudflare DNS
sudo ./ft_ping 9.9.9.9          # Quad9 DNS
```
**Expected Output (example for 8.8.8.8):**
```
PING 8.8.8.8 (8.8.8.8): 56 data bytes
64 bytes from 8.8.8.8: icmp_seq=0 ttl=119 time=146.167 ms
64 bytes from 8.8.8.8: icmp_seq=1 ttl=119 time=74.458 ms
64 bytes from 8.8.8.8: icmp_seq=2 ttl=119 time=580.674 ms
^C
--- 8.8.8.8 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 74.458/267.100/580.674/223.655 ms
```
**Expected:** Replies with variable latency (10-600ms depending on location/network), 0% packet loss, TTL varies (typically 50-120), runs indefinitely until Ctrl-C, exit code 0

### Test 3: Local Gateway
```bash
# Find your gateway **first**
ip route | grep default
# Then ping it
sudo ./ft_ping 10.0.1.1  # (use your actual gateway IP)
```
**Expected Output (example):**
```
PING 10.0.1.1 (10.0.1.1): 56 data bytes
64 bytes from 10.0.1.1: icmp_seq=0 ttl=255 time=14.482 ms
64 bytes from 10.0.1.1: icmp_seq=1 ttl=255 time=6.829 ms
64 bytes from 10.0.1.1: icmp_seq=2 ttl=255 time=1.350 ms
^C
--- 10.0.1.1 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 1.350/7.554/14.482/5.386 ms
```
**Expected:** Fast replies (typically 1-15ms), 0% packet loss, TTL usually 255 or 64, runs indefinitely until Ctrl-C, exit code 0

---

## Timeout Tests

### Test 4: Non-routable IP (No Reply)
```bash
sudo ./ft_ping 192.0.2.1        # TEST-NET-1 (RFC 5737)
sudo ./ft_ping 198.51.100.1     # TEST-NET-2
sudo ./ft_ping 203.0.113.1      # TEST-NET-3
```
**Expected Output (inetutils-2.0):**
```
PING 192.0.2.1 (192.0.2.1): 56 data bytes
(no replies, waits indefinitely - use Ctrl-C to stop)
^C
--- 192.0.2.1 ping statistics ---
10 packets transmitted, 0 packets received, 100% packet loss
```
**Expected:** No replies shown, ping waits indefinitely (sends packets every ~1 second), 100% packet loss when interrupted with Ctrl-C, exit code 0 (interrupted) or 1 (if using -c option)

**Note:** Unlike some implementations, inetutils ping does NOT have a per-packet timeout by default. It will continue sending packets indefinitely until stopped.

### Test 5: Valid but Filtered Host
```bash
sudo ./ft_ping 192.168.255.254  # Likely doesn't exist in your network
```
**Expected:** No replies, waits indefinitely, 100% packet loss when stopped

### Test 6: Private Network (if not on it)
```bash
sudo ./ft_ping 10.255.255.254   # Private network you're not on
```
**Expected:** Either no replies (waits indefinitely) or ICMP unreachable from your router (if router responds with Type 3 error)

---

## ICMP Error Tests

### Test 7: Network Unreachable
```bash
# Add a fake route that goes nowhere
sudo ip route add 192.0.2.0/24 via 10.0.1.254  # Non-existent gateway

# Ping it
sudo ./ft_ping 192.0.2.5

# Cleanup
sudo ip route del 192.0.2.0/24
```
**Expected Output (inetutils-2.0):**
```
/home/tfregni/Desktop/42/advanced/ft_ping/bin/ping: sending packet: Operation not permitted
```
**Expected:** No packets sent, error message, exit code 1

### Test 8: Host Unreachable (Using ARP Failure)
```bash
sudo ./ft_ping -v 10.0.1.254
```
**Expected Output (inetutils-2.0):**
```
PING 10.0.1.254 (10.0.1.254): 56 data bytes, id 0x8a0e = 35342
92 bytes from tfregni-ryzen (10.0.1.50): Destination Host Unreachable
IP Hdr Dump:
 4500 0054 257f 4000 4001 fdfa 0a00 0132 0a00 01fe 
Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data
 4  5  00 0054 257f   2 0000  40  01 fdfa 10.0.1.50  10.0.1.254 
ICMP: type 8, code 0, size 64, id 0x8a0e, seq 0x0000
... (repeats for each packet)
--- 10.0.1.254 ping statistics ---
3 packets transmitted, 0 packets received, 100% packet loss
```
**Expected:** Each failed packet prints a verbose error and packet dump, exit code 1

### Test 9: Simulate Port Unreachable (Different Protocol)
This is not shown by ping itself, but can be observed with tcpdump or Wireshark.

---

## Network Simulation Tests

### Test 10: Packet Loss Simulation
```bash
# Add 30% packet loss
sudo tc qdisc add dev lo root netem loss 30%

# Test
sudo ./ft_ping 127.0.0.1

# Cleanup
sudo tc qdisc del dev lo root
```
**Expected Output (example with -c 10):**
```
PING 127.0.0.1 (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.045 ms
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.053 ms
64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.050 ms
64 bytes from 127.0.0.1: icmp_seq=3 ttl=64 time=0.060 ms
64 bytes from 127.0.0.1: icmp_seq=5 ttl=64 time=0.044 ms
64 bytes from 127.0.0.1: icmp_seq=7 ttl=64 time=0.051 ms
--- 127.0.0.1 ping statistics ---
10 packets transmitted, 6 packets received, 40% packet loss
round-trip min/avg/max/stddev = 0.044/0.050/0.060/0.000 ms
```
**Expected:** Approximately 30% packet loss (may vary), missing sequence numbers (e.g., 4, 6, 8, 9), exit code 1 with -c option

### Test 11: Latency Simulation
```bash
# Add 200ms delay
sudo tc qdisc add dev lo root netem delay 200ms

# Test
sudo ./ft_ping 127.0.0.1

# Cleanup
sudo tc qdisc del dev lo root
```
**Expected Output (example with -c 5):**
```
PING 127.0.0.1 (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=400.091 ms
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=400.073 ms
64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=400.077 ms
64 bytes from 127.0.0.1: icmp_seq=3 ttl=64 time=400.073 ms
64 bytes from 127.0.0.1: icmp_seq=4 ttl=64 time=400.077 ms
--- 127.0.0.1 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max/stddev = 400.073/400.078/400.091/0.000 ms
```
**Expected:** ~400ms response times (200ms each direction), very consistent timing, 0% packet loss

### Test 12: Jitter and Packet Reordering
```bash
# Add delay with variation
sudo tc qdisc add dev lo root netem delay 100ms 50ms

# Test
sudo ./ft_ping 127.0.0.1

# Cleanup
sudo tc qdisc del dev lo root
```
**Expected Output (example with -c 10):**
```
PING 127.0.0.1 (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=212.591 ms
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=180.117 ms
64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=138.737 ms
64 bytes from 127.0.0.1: icmp_seq=3 ttl=64 time=236.413 ms
64 bytes from 127.0.0.1: icmp_seq=4 ttl=64 time=123.022 ms
...
--- 127.0.0.1 ping statistics ---
10 packets transmitted, 10 packets received, 0% packet loss
round-trip min/avg/max/stddev = 123.022/188.696/239.679/35.660 ms
```
**Expected:** Highly variable response times (roughly 100-300ms range), significant stddev (30-50ms), 0% packet loss

### Test 13: Complete Blackhole
```bash
# Block ICMP replies using iptables
sudo iptables -A OUTPUT -p icmp --icmp-type echo-reply -j DROP

# Test
sudo ./ft_ping 127.0.0.1

# Cleanup
sudo iptables -D OUTPUT -p icmp --icmp-type echo-reply -j DROP
```
**Expected Output (example with -c 5):**
```
PING 127.0.0.1 (127.0.0.1): 56 data bytes
--- 127.0.0.1 ping statistics ---
5 packets transmitted, 0 packets received, 100% packet loss
```
**Expected:** No replies shown, 100% packet loss, exit code 1 (with -c option)

---

## Edge Case Tests

### Test 14: TTL Too Low
Standard ping with TTL=1 to see what should happen:
```bash
# Standard ping with TTL=1
ping -t 1 8.8.8.8
```
**Expected (standard ping):** ICMP Type 11, Code 0 (Time Exceeded) from first hop router

**Note:** You'd need to implement TTL option in ft_ping to test this in your code.

### Test 15: Rapid Fire (Stress Test)
Modify your ping loop to remove the `sleep(1)` temporarily:
```bash
sudo ./ft_ping 127.0.0.1
```
**Expected:** High packet rate, system stays responsive, all packets succeed

### Test 16: Large Payload (If Implemented)
If you add support for different packet sizes:
```bash
sudo ./ft_ping -s 1000 127.0.0.1  # 1000 byte payload
```
**Expected:** Larger packets work correctly

### Test 17: Invalid Destination
```bash
# These should fail during address parsing, not during ping
sudo ./ft_ping 999.999.999.999
sudo ./ft_ping invalid.test.example
```
**Expected Output (inetutils-2.0):**
```
/path/to/ping: unknown host
```
**Expected:** Error message immediately (no packets sent), exit code 1, program terminates before attempting to ping

---

## Network Namespace Tests (Advanced)

### Test 18: Isolated Network Environment
```bash
sudo ip netns add test_ns
sudo ip netns exec test_ns ./ft_ping 8.8.8.8
sudo ip netns del test_ns
```
**Expected Output (inetutils-2.0):**
```
PING 8.8.8.8 (8.8.8.8): 56 data bytes
/home/tfregni/Desktop/42/advanced/ft_ping/bin/ping: sending packet: Network is unreachable
```
**Expected:** No packets sent, error message, exit code 1

### Test 19: Namespace with Veth Pair
```bash
# Create namespace and veth pair
sudo ip netns add test_ns
sudo ip link add veth0 type veth peer name veth1
sudo ip link set veth1 netns test_ns

# Configure addresses
sudo ip addr add 10.200.1.1/24 dev veth0
sudo ip link set veth0 up
sudo ip netns exec test_ns ip addr add 10.200.1.2/24 dev veth1
sudo ip netns exec test_ns ip link set veth1 up
sudo ip netns exec test_ns ip link set lo up

# Test from main namespace
sudo ./ft_ping 10.200.1.2

# Test from isolated namespace
sudo ip netns exec test_ns ./ft_ping 10.200.1.1

# Cleanup
sudo ip netns del test_ns
sudo ip link del veth0
```
**Expected Output (inetutils-2.0):**
```
PING 10.200.1.2 (10.200.1.2): 56 data bytes
64 bytes from 10.200.1.2: icmp_seq=0 ttl=64 time=0.090 ms
64 bytes from 10.200.1.2: icmp_seq=1 ttl=64 time=0.056 ms
64 bytes from 10.200.1.2: icmp_seq=2 ttl=64 time=0.040 ms
--- 10.200.1.2 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.040/0.062/0.090/0.000 ms

PING 10.200.1.1 (10.200.1.1): 56 data bytes
64 bytes from 10.200.1.1: icmp_seq=0 ttl=64 time=0.060 ms
64 bytes from 10.200.1.1: icmp_seq=1 ttl=64 time=0.052 ms
64 bytes from 10.200.1.1: icmp_seq=2 ttl=64 time=0.053 ms
--- 10.200.1.1 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.052/0.055/0.060/0.000 ms
```
**Expected:** Successful pings between namespaces, low latency, 0% packet loss

---

## Comparison Testing

### Test 20: Side-by-Side Comparison
```bash
# Run both simultaneously in different terminals
# Terminal 1 (inetutils-2.0 reference):
/path/to/inetutils/bin/ping -c 10 8.8.8.8

# Terminal 2 (your implementation):
sudo ./ft_ping 8.8.8.8  # (stop after 10 packets with Ctrl-C)
```

**Compare the following in inetutils-2.0 output:**

**Header format:**
```
PING 8.8.8.8 (8.8.8.8): 56 data bytes
```

**Per-packet format:**
```
64 bytes from 8.8.8.8: icmp_seq=0 ttl=119 time=146.167 ms
```

**Statistics format (after Ctrl-C or -c completion):**
```
--- 8.8.8.8 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 74.458/267.100/580.674/223.655 ms
```

**Key Points:**
- Sequence numbers start at 0 (not 1)
- TTL values match between implementations
- Response times should be similar (within reasonable variance)
- Packet loss statistics match
- Statistics line format: `round-trip min/avg/max/stddev = ...` with values in ms
- Exit code 0 on success (all or some packets received)
- Exit code 1 on failure (no packets received or invalid host)

### Test 21: Capture with tcpdump
```bash
# Start capture
sudo tcpdump -i any icmp -n -vv > ping_capture.txt &
TCPDUMP_PID=$!

# Run your ping
sudo ./ft_ping 8.8.8.8

# Stop after a few packets (Ctrl-C)
# Stop tcpdump
sudo kill $TCPDUMP_PID

# Examine capture
cat ping_capture.txt
```

**Verify:**
- ICMP Echo Request has correct type (8)
- ICMP Echo Reply has correct type (0)
- ID field matches your PID
- Sequence numbers increment correctly
- Checksums are valid

### Test 22: Wireshark Analysis
```bash
# Start wireshark
sudo wireshark &

# Filter: icmp
# Run your ping
sudo ./ft_ping 8.8.8.8

# In Wireshark, verify:
# - All fields are correctly formatted
# - Checksums are valid (green checkmark)
# - Sequence numbers increment
# - ID is consistent
```

---

## Automated Test Script

Create `test_ft_ping.sh`:

```bash
#!/bin/bash

echo "=== ft_ping Test Suite ==="
echo

# Test 1: Localhost
echo "Test 1: Localhost (expect success)"
timeout 5 sudo ./ft_ping 127.0.0.1 &
PING_PID=$!
sleep 3
sudo kill -INT $PING_PID
wait $PING_PID 2>/dev/null
echo

# Test 2: Public DNS
echo "Test 2: Public DNS (expect success)"
timeout 5 sudo ./ft_ping 8.8.8.8 &
PING_PID=$!
sleep 3
sudo kill -INT $PING_PID
wait $PING_PID 2>/dev/null
echo

# Test 3: Non-routable (expect no replies, but ping continues)
echo "Test 3: Non-routable IP (expect no replies)"
timeout 10 sudo ./ft_ping 192.0.2.1 &
PING_PID=$!
sleep 8
sudo kill -INT $PING_PID
wait $PING_PID 2>/dev/null
echo "Note: Should show 100% packet loss but exit code may be 0 (interrupted)"
echo

# Test 4: Invalid address (expect immediate error with exit code 1)
echo "Test 4: Invalid address (expect immediate error)"
sudo ./ft_ping 999.999.999.999
EXIT_CODE=$?
echo "Exit code: $EXIT_CODE (expected: 1)"
echo

echo "=== Tests Complete ==="
```

Run with:
```bash
chmod +x test_ft_ping.sh
./test_ft_ping.sh
```

**Expected Results:**
- Test 1 & 2: Should show replies and statistics, exit cleanly
- Test 3: No replies shown, 100% packet loss in statistics
- Test 4: Error message "unknown host", exit code 1

---

## Quick Reference: Expected ICMP Types

| Type | Code | Meaning | How to Test |
|------|------|---------|-------------|
| 0 | 0 | Echo Reply | Normal successful ping |
| 3 | 0 | Network Unreachable | Ping unreachable network |
| 3 | 1 | Host Unreachable | Ping non-existent host on local network |
| 3 | 3 | Port Unreachable | Not applicable to ICMP Echo |
| 8 | 0 | Echo Request | What you send |
| 11 | 0 | TTL Exceeded | Use ping -t 1 to distant host |

---

## Debugging Tips

### Check Raw Socket Permissions
```bash
# Verify raw socket capability
sudo getcap ./ft_ping
# Should show: cap_net_raw+ep

# Or just use sudo
sudo ./ft_ping 8.8.8.8
```

### Monitor All ICMP Traffic
```bash
# In separate terminal
sudo tcpdump -i any -n icmp
```

### Check Routing
```bash
# See where packets would go
ip route get 8.8.8.8
```

### Verify Firewall Rules
```bash
# Check if ICMP is blocked
sudo iptables -L -n -v | grep icmp
```

---

## Common Issues and Solutions

### Issue: "Operation not permitted"
**Solution:** Run with sudo or add capabilities:
```bash
sudo setcap cap_net_raw+ep ./ft_ping
```

### Issue: Always timing out
**Solution:** Check firewall, routing, and that you're handling replies correctly

### Issue: Seeing lots of Type 3 messages
**Solution:** Filter by source IP and ICMP ID to only process your packets

### Issue: Wrong packet counts
**Solution:** Only count ICMP_ECHOREPLY with matching ID and source IP

---

## Advanced: Create a Test Server

Python script to respond to pings with errors:

```python
#!/usr/bin/env python3
from scapy.all import *

def send_icmp_error(pkt):
    if ICMP in pkt and pkt[ICMP].type == 8:  # Echo Request
        # Send Host Unreachable instead of Echo Reply
        ip = IP(src=pkt[IP].dst, dst=pkt[IP].src)
        icmp = ICMP(type=3, code=1)  # Host Unreachable
        # Include original IP header + 8 bytes
        payload = bytes(pkt[IP])[:28]
        send(ip/icmp/payload)
        print(f"Sent Host Unreachable to {pkt[IP].src}")

sniff(filter="icmp[icmptype] == 8", prn=send_icmp_error)
```

Run with: `sudo python3 test_server.py`

---

## Summary Checklist

- [ ] Test localhost (fast, reliable, < 0.1ms)
- [ ] Test public DNS (variable latency, 10-600ms depending on network)
- [ ] Test non-routable IP (no replies, runs indefinitely without per-packet timeout)
- [ ] Test with packet loss simulation
- [ ] Test with latency simulation
- [ ] Compare output format with inetutils-2.0 ping
- [ ] Verify sequence numbers start at 0
- [ ] Verify statistics format: `round-trip min/avg/max/stddev = ...`
- [ ] Verify packet format with tcpdump/Wireshark
- [ ] Test invalid addresses (should exit with code 1, message "unknown host")
- [ ] Test statistics are accurate (min/avg/max/stddev calculated correctly)
- [ ] Test signal handling (Ctrl-C shows statistics and exits cleanly)
- [ ] Check for memory leaks (valgrind)
- [ ] Verify exit codes: 0 on success/interrupt, 1 on error/100% loss with -c

---

Good luck testing your ft_ping! 🚀
