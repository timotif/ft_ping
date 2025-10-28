# ft_ping Testing Guide

A comprehensive guide for testing various ping scenarios and error conditions.

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
**Expected:** Very fast replies (< 1ms), 0% packet loss, TTL=64

### Test 2: Public DNS Servers
```bash
sudo ./ft_ping 8.8.8.8          # Google DNS
sudo ./ft_ping 1.1.1.1          # Cloudflare DNS
sudo ./ft_ping 9.9.9.9          # Quad9 DNS
```
**Expected:** Fast replies (10-100ms depending on location), 0% packet loss, TTL varies

### Test 3: Local Gateway
```bash
# Find your gateway **first**
ip route | grep default
# Then ping it
sudo ./ft_ping 192.168.1.1  # (use your actual gateway IP)
```
**Expected:** Very fast replies (< 5ms), 0% packet loss

---

## Timeout Tests

### Test 4: Non-routable IP (Timeout)
```bash
sudo ./ft_ping 192.0.2.1        # TEST-NET-1 (RFC 5737)
sudo ./ft_ping 198.51.100.1     # TEST-NET-2
sudo ./ft_ping 203.0.113.1      # TEST-NET-3
```
**Expected:** No replies, timeout after 3 seconds per packet, 100% packet loss

### Test 5: Valid but Filtered Host
```bash
sudo ./ft_ping 192.168.255.254  # Likely doesn't exist in your network
```
**Expected:** Timeout, 100% packet loss

### Test 6: Private Network (if not on it)
```bash
sudo ./ft_ping 10.255.255.254   # Private network you're not on
```
**Expected:** Either timeout or ICMP unreachable from your router

---

## ICMP Error Tests

### Test 7: Network Unreachable
```bash
# Add a fake route that goes nowhere
sudo ip route add 192.0.2.0/24 via 192.168.1.254  # Non-existent gateway

# Ping it
sudo ./ft_ping 192.0.2.5

# Cleanup
sudo ip route del 192.0.2.0/24
```
**Expected:** ICMP Type 3, Code 0 (Network Unreachable) from your gateway

### Test 8: Host Unreachable (Using ARP Failure)
```bash
# Ping a host on your local subnet that doesn't exist
# Find your subnet first:
ip addr show | grep inet

# If you're on 192.168.1.0/24, try:
sudo ./ft_ping 192.168.1.254
```
**Expected:** Either timeout or ICMP Type 3, Code 1 (Host Unreachable)

### Test 9: Simulate Port Unreachable (Different Protocol)
This is trickier since ping uses ICMP, but you can observe it by:
```bash
# In one terminal, monitor ICMP with your ft_ping verbose mode
sudo ./ft_ping -v 8.8.8.8

# In another terminal, send UDP to a random port
nc -u 8.8.8.8 12345
```
**Expected:** You'll see ICMP Type 3, Code 3 messages (but not related to your ping)

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
**Expected:** ~30% packet loss reported

### Test 11: Latency Simulation
```bash
# Add 200ms delay
sudo tc qdisc add dev lo root netem delay 200ms

# Test
sudo ./ft_ping 127.0.0.1

# Cleanup
sudo tc qdisc del dev lo root
```
**Expected:** ~200ms response times (plus variance)

### Test 12: Jitter and Packet Reordering
```bash
# Add delay with variation
sudo tc qdisc add dev lo root netem delay 100ms 50ms

# Test
sudo ./ft_ping 127.0.0.1

# Cleanup
sudo tc qdisc del dev lo root
```
**Expected:** Variable response times (50-150ms range)

### Test 13: Complete Blackhole
```bash
# Block ICMP replies using iptables
sudo iptables -A OUTPUT -p icmp --icmp-type echo-reply -j DROP

# Test
sudo ./ft_ping 127.0.0.1

# Cleanup
sudo iptables -D OUTPUT -p icmp --icmp-type echo-reply -j DROP
```
**Expected:** 100% packet loss, timeout on every packet

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
**Expected:** Error message before attempting to ping

---

## Network Namespace Tests (Advanced)

### Test 18: Isolated Network Environment
```bash
# Create isolated network namespace
sudo ip netns add test_ns

# Run ping in isolated namespace (no network access)
sudo ip netns exec test_ns ./ft_ping 8.8.8.8

# Cleanup
sudo ip netns del test_ns
```
**Expected:** Network unreachable error

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
**Expected:** Successful pings between namespaces

---

## Comparison Testing

### Test 20: Side-by-Side Comparison
```bash
# Run both simultaneously in different terminals
# Terminal 1:
ping -c 10 8.8.8.8

# Terminal 2:
sudo ./ft_ping 8.8.8.8  # (stop after 10 packets with Ctrl-C)
```

**Compare:**
- Sequence numbers match
- TTL values match
- Response times are similar
- Packet loss statistics match
- Summary statistics format

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

# Test 3: Non-routable (expect timeout)
echo "Test 3: Non-routable IP (expect timeout)"
timeout 10 sudo ./ft_ping 192.0.2.1 &
PING_PID=$!
sleep 8
sudo kill -INT $PING_PID
wait $PING_PID 2>/dev/null
echo

# Test 4: Invalid address (expect error)
echo "Test 4: Invalid address (expect immediate error)"
sudo ./ft_ping 999.999.999.999
echo

echo "=== Tests Complete ==="
```

Run with:
```bash
chmod +x test_ft_ping.sh
./test_ft_ping.sh
```

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

- [ ] Test localhost (fast, reliable)
- [ ] Test public DNS (normal latency)
- [ ] Test non-routable IP (timeout)
- [ ] Test with packet loss simulation
- [ ] Test with latency simulation
- [ ] Compare output with standard ping
- [ ] Verify packet format with tcpdump/Wireshark
- [ ] Test invalid addresses
- [ ] Test statistics are accurate
- [ ] Test signal handling (Ctrl-C)
- [ ] Check for memory leaks (valgrind)

---

Good luck testing your ft_ping! 🚀
