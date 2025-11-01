# Bitmap Reference Card for ft_ping

Quick reference for implementing bitmap-based duplicate detection.

---

## Core Concepts

### What is a Bitmap?
An array where each **bit** (not byte) represents a boolean flag.

**Memory savings:**
- Array of char: 65,536 bytes (1 byte per sequence)
- Bitmap: 8,192 bytes (1 bit per sequence)
- **Savings: 87.5%**

### Mapping Sequence to Bitmap

```
For sequence number N (0-65535):
┌─────────────────────────────────────┐
│ Byte Index = N / 8  (or N >> 3)    │
│ Bit Offset = N % 8  (or N & 7)     │
└─────────────────────────────────────┘

Example: Sequence 42
  42 / 8 = 5  → bitmap[5]
  42 % 8 = 2  → bit position 2

  bitmap[5] bit layout:
  [7][6][5][4][3][2][1][0]
                   ↑
              Sequence 42
```

---

## The Three Operations

### 1. SET BIT (Mark as Received)

```c
void bitmap_set(uint8_t *bitmap, uint16_t seq)
{
    bitmap[seq >> 3] |= (1 << (seq & 7));
}
```

**How it works:**
1. `seq >> 3`: Find which byte (divide by 8)
2. `seq & 7`: Find which bit in that byte (modulo 8)
3. `1 << offset`: Create mask with only that bit ON
4. `|=`: OR it in (turns bit ON, preserves others)

**Visual:**
```
Before: bitmap[5] = 00000000
Set seq 42 (bit 2):
  mask = 1 << 2 = 00000100
  result = 00000000 | 00000100 = 00000100
After:  bitmap[5] = 00000100
```

### 2. TEST BIT (Check for Duplicate)

```c
int bitmap_test(uint8_t *bitmap, uint16_t seq)
{
    return (bitmap[seq >> 3] & (1 << (seq & 7))) != 0;
}
```

**How it works:**
1. Create mask (same as SET)
2. `&`: AND with byte (isolates that bit)
3. `!= 0`: Non-zero means bit was set

**Visual:**
```
bitmap[5] = 00000100 (bit 2 is ON)

Test seq 42 (bit 2):
  mask = 1 << 2 = 00000100
  00000100 & 00000100 = 00000100 (non-zero)
  → Return 1 (DUPLICATE!)

Test seq 43 (bit 3):
  mask = 1 << 3 = 00001000
  00000100 & 00001000 = 00000000 (zero)
  → Return 0 (not duplicate)
```

### 3. CLEAR BIT (Reset)

```c
void bitmap_clear(uint8_t *bitmap, uint16_t seq)
{
    bitmap[seq >> 3] &= ~(1 << (seq & 7));
}
```

**How it works:**
1. Create mask
2. `~`: Invert mask (all bits flip)
3. `&=`: AND with inverted mask (clears that bit only)

**Visual:**
```
Before: bitmap[5] = 00000100 (bit 2 is ON)
Clear seq 42:
  mask = 1 << 2 = 00000100
  ~mask = 11111011  (NOT flips all bits)
  00000100 & 11111011 = 00000000
After:  bitmap[5] = 00000000
```

---

## Bit Operations Cheat Sheet

| Operation | Symbol | Truth Table | Use Case |
|-----------|--------|-------------|----------|
| **AND** | `&` | `1 & 1 = 1`, else `0` | Test if bit is set |
| **OR** | `\|` | `0 \| 0 = 0`, else `1` | Set bit to 1 |
| **NOT** | `~` | `~0 = 1`, `~1 = 0` | Invert all bits |
| **XOR** | `^` | `same = 0`, `diff = 1` | Toggle bits |
| **Left Shift** | `<<` | `n << 1` doubles n | Create masks |
| **Right Shift** | `>>` | `n >> 1` halves n | Divide by powers of 2 |

**Creating masks:**
```c
1 << 0  =  00000001  (bit 0 mask)
1 << 3  =  00001000  (bit 3 mask)
1 << 7  =  10000000  (bit 7 mask)
```

---

## Integration into ft_ping

### Step 1: Add to structure (ft_ping.h)

```c
typedef struct s_app
{
    // ... existing fields ...
    uint8_t dup_bitmap[8192];  // Duplicate detection bitmap
} t_app;
```

### Step 2: Initialize bitmap

```c
// In main() or init function:
memset(app.dup_bitmap, 0, 8192);
```

### Step 3: Modify packet handling (ping.c)

```c
void ping_success(t_app *app, uint16_t seq, ...)
{
    // CHECK for duplicate FIRST
    if (bitmap_test(app->dup_bitmap, seq)) {
        printf("... (DUP!)\n");
        return;  // Don't count duplicates
    }

    // MARK as received
    bitmap_set(app->dup_bitmap, seq);

    // Normal processing
    printf("... time=%.3f ms\n", rtt);
    app->rcv_packets++;  // Only increment for new packets
}
```

---

## Common Mistakes

### ❌ WRONG: Testing AFTER setting
```c
bitmap_set(bitmap, seq);
if (bitmap_test(bitmap, seq)) {  // Always true!
    // ...
}
```

### ✅ CORRECT: Testing BEFORE setting
```c
if (bitmap_test(bitmap, seq)) {  // Check first
    return;  // Duplicate
}
bitmap_set(bitmap, seq);  // Then mark
```

---

### ❌ WRONG: Using assignment (overwrites byte)
```c
bitmap[idx] = (1 << offset);  // DESTROYS other bits!
```

### ✅ CORRECT: Using OR (preserves other bits)
```c
bitmap[idx] |= (1 << offset);  // Sets one bit only
```

---

### ❌ WRONG: Counting duplicates in statistics
```c
app->rcv_packets++;  // Before checking duplicate
if (is_duplicate) return;
```

### ✅ CORRECT: Only count non-duplicates
```c
if (is_duplicate) return;  // Check first
app->rcv_packets++;  // Then increment
```

---

## Performance Notes

**Why shift/AND instead of division/modulo?**

```c
// Slower (may use division instruction):
byte_idx = seq / 8;
bit_off = seq % 8;

// Faster (guaranteed bit operations):
byte_idx = seq >> 3;   // Right shift by 3 = divide by 8
bit_off = seq & 7;     // AND with 7 = modulo 8
```

**Math proof:**
- `42 >> 3 = 5` (same as `42 / 8`)
- `42 & 7 = 2` (same as `42 % 8`)

**Why this works:**
- Dividing by 8 = shifting right 3 positions (8 = 2³)
- Modulo 8 = keeping only the last 3 bits (7 = 0b111)

---

## Testing Your Implementation

### Test 1: Basic duplicate detection
```bash
# Send duplicate by running tcpreplay or similar
# Should see "(DUP!)" suffix on duplicate packets
```

### Test 2: Sequence wrap-around
```bash
# Run ping long enough to overflow uint16_t
# May need to clear bitmap when seq wraps to 0
```

### Test 3: Edge cases
```c
// Test boundary sequences:
test_seq_0();      // First sequence
test_seq_7();      // Last bit in byte 0
test_seq_8();      // First bit in byte 1
test_seq_65535();  // Last possible sequence
```

---

## Visual Memory Layout

```
Bitmap array (8192 bytes total):

  Index:     0        1        2      ...     8191
          ┌────────┬────────┬────────┬───┬────────┐
  Bytes:  │ 8 bits │ 8 bits │ 8 bits │...│ 8 bits │
          └────────┴────────┴────────┴───┴────────┘
          ↑                              ↑
       Seqs 0-7                    Seqs 65528-65535

Byte 0 detail (holds sequences 0-7):
  Bit:  7   6   5   4   3   2   1   0
       ┌───┬───┬───┬───┬───┬───┬───┬───┐
  Seq: │ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
       └───┴───┴───┴───┴───┴───┴───┴───┘
```

---

## Quick Reference: Bitmap Functions

```c
// Initialize (call once at startup)
void bitmap_init(uint8_t *bitmap)
{
    memset(bitmap, 0, 8192);
}

// Mark as received (after checking for duplicate)
void bitmap_set(uint8_t *bitmap, uint16_t seq)
{
    bitmap[seq >> 3] |= (1 << (seq & 7));
}

// Check if already received (before processing)
int bitmap_test(uint8_t *bitmap, uint16_t seq)
{
    return (bitmap[seq >> 3] & (1 << (seq & 7))) != 0;
}

// Clear (optional - for testing or sequence wrap)
void bitmap_clear(uint8_t *bitmap, uint16_t seq)
{
    bitmap[seq >> 3] &= ~(1 << (seq & 7));
}
```

---

## Files Included in This Tutorial

1. **bitmap_tutorial.c** - Comprehensive demonstrations
2. **bitmap_exercises.c** - Hands-on practice problems
3. **bitmap_integration_guide.c** - Step-by-step integration
4. **BITMAP_REFERENCE.md** - This file (quick reference)

**To run:**
```bash
gcc bitmap_tutorial.c -o tutorial && ./tutorial
gcc bitmap_exercises.c -o exercises && ./exercises
```

---

## Further Reading

- RFC 1071: Internet Checksum (uses similar bit operations)
- RFC 792: ICMP specification
- Linux kernel: `include/linux/bitmap.h` (production implementation)
- "Hacker's Delight" by Henry S. Warren (bit manipulation techniques)

---

## Summary

**Remember the three operations:**
1. **SET**: `bitmap[byte] |= (1 << bit)` - Mark as received
2. **TEST**: `(bitmap[byte] & (1 << bit)) != 0` - Check duplicate
3. **CLEAR**: `bitmap[byte] &= ~(1 << bit)` - Reset

**Integration pattern:**
```c
if (bitmap_test(dup_bitmap, seq))  // Already received?
    return;                        // Skip duplicate
bitmap_set(dup_bitmap, seq);       // Mark as received
process_packet();                  // Handle new packet
```

**Cost:** 8 KB of memory to track all 65,536 possible ICMP sequences.

Good luck with your ft_ping implementation! 🚀
