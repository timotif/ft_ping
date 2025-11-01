/*
** BITMAP INTEGRATION GUIDE FOR FT_PING
** =====================================
**
** This file shows how to integrate bitmap-based duplicate detection
** into your ft_ping project.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// ============================================================================
// STEP 1: Add to ft_ping.h (in t_app structure)
// ============================================================================

/*
typedef struct s_app
{
    // ... existing fields ...

    // Add this:
    uint8_t dup_bitmap[8192];  // Bitmap for duplicate detection (65536 bits)

} t_app;
*/

// ============================================================================
// STEP 2: Add bitmap utility functions (new file: bitmap.c or in utils.c)
// ============================================================================

/**
 * Initialize bitmap to all zeros
 * Call this once during app initialization
 */
void bitmap_init(uint8_t *bitmap)
{
    memset(bitmap, 0, 8192);
}

/**
 * Mark sequence number as received
 * Call this when you receive a valid echo reply
 */
void bitmap_set(uint8_t *bitmap, uint16_t seq)
{
    uint16_t byte_index = seq >> 3;  // seq / 8
    uint8_t  bit_offset = seq & 7;   // seq % 8

    bitmap[byte_index] |= (1 << bit_offset);
}

/**
 * Check if sequence number was already received
 * Call this BEFORE processing a received packet
 * Returns: 1 if duplicate, 0 if new
 */
int bitmap_test(uint8_t *bitmap, uint16_t seq)
{
    uint16_t byte_index = seq >> 3;
    uint8_t  bit_offset = seq & 7;

    return (bitmap[byte_index] & (1 << bit_offset)) != 0;
}

/**
 * Clear a sequence number (optional - for testing)
 * You probably won't need this in production
 */
void bitmap_clear(uint8_t *bitmap, uint16_t seq)
{
    uint16_t byte_index = seq >> 3;
    uint8_t  bit_offset = seq & 7;

    bitmap[byte_index] &= ~(1 << bit_offset);
}

// ============================================================================
// STEP 3: Integrate into your ping flow
// ============================================================================

/*
** MODIFY: ping_success() in ping.c
**
** Current flow:
**   1. Receive packet
**   2. Extract sequence number
**   3. Print success message
**   4. Update stats
**
** NEW flow with duplicate detection:
**   1. Receive packet
**   2. Extract sequence number
**   3. CHECK if duplicate  <-- NEW
**   4. If duplicate, print warning and return early
**   5. MARK as received    <-- NEW
**   6. Print success message
**   7. Update stats
*/

#if 0  // Pseudocode - adapt to your actual ping_success() function

void ping_success(t_app *app, uint16_t seq, struct timeval *rtt)
{
    // Check for duplicate BEFORE processing
    if (bitmap_test(app->dup_bitmap, seq)) {
        // This is a duplicate packet!
        printf("%lu bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms (DUP!)\n",
               PACKET_SIZE, app->dest_ip, seq, ttl, rtt_ms);
        return;  // Don't count duplicates in statistics
    }

    // Mark as received (first time we've seen this sequence)
    bitmap_set(app->dup_bitmap, seq);

    // Normal success handling (existing code)
    printf("%lu bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
           PACKET_SIZE, app->dest_ip, seq, ttl, rtt_ms);

    app->rcv_packets++;  // Only increment for non-duplicates
    // ... update min/max/avg stats ...
}

#endif

// ============================================================================
// STEP 4: Initialize bitmap in main()
// ============================================================================

/*
** MODIFY: init_app() or main() in ft_ping.c
*/

#if 0  // Pseudocode

int main(int argc, char **argv)
{
    t_app app;

    // ... existing initialization ...

    // Add this:
    bitmap_init(app.dup_bitmap);

    // ... rest of program ...
}

#endif

// ============================================================================
// STEP 5: Test cases to verify your implementation
// ============================================================================

void test_duplicate_detection(void)
{
    uint8_t bitmap[8192];
    bitmap_init(bitmap);

    printf("=== DUPLICATE DETECTION TEST ===\n\n");

    // Simulate receiving packets in this order:
    uint16_t packets[] = {1, 2, 3, 2, 4, 5, 3, 1, 6};

    for (int i = 0; i < 9; i++) {
        uint16_t seq = packets[i];

        if (bitmap_test(bitmap, seq)) {
            printf("Seq %u: DUPLICATE - already received\n", seq);
        } else {
            printf("Seq %u: NEW - first time seeing this\n", seq);
            bitmap_set(bitmap, seq);
        }
    }

    printf("\nExpected output:\n");
    printf("  Seq 1: NEW\n");
    printf("  Seq 2: NEW\n");
    printf("  Seq 3: NEW\n");
    printf("  Seq 2: DUPLICATE <--\n");
    printf("  Seq 4: NEW\n");
    printf("  Seq 5: NEW\n");
    printf("  Seq 3: DUPLICATE <--\n");
    printf("  Seq 1: DUPLICATE <--\n");
    printf("  Seq 6: NEW\n");
}

// ============================================================================
// STEP 6: Advanced - Handle sequence number wrap-around (optional)
// ============================================================================

/*
** ICMP sequence numbers are uint16_t (0-65535).
** After 65535, they wrap to 0.
**
** Your bitmap handles this automatically because:
**   - Bitmap size is exactly 65536 bits (uint16_t range)
**   - seq 0 maps to byte[0], bit 0
**   - seq 65535 maps to byte[8191], bit 7
**   - When seq wraps to 0, bitmap position resets too
**
** HOWEVER: If you receive packet 65535, then packet 0, the second
** will be marked as duplicate (because 0 was received in a previous cycle).
**
** Solutions:
**   1. Clear bitmap when sequence wraps (detect when new_seq < old_seq)
**   2. Use a sliding window (more complex)
**   3. Accept this limitation (ping usually doesn't run that long)
*/

#if 0  // Pseudocode - example of handling wrap-around

void handle_sequence_wrap(t_app *app, uint16_t new_seq)
{
    static uint16_t last_seq = 0;

    // Detect wrap-around: sequence decreased significantly
    if (new_seq < last_seq && (last_seq - new_seq) > 32768) {
        printf("Sequence number wrapped, clearing duplicate bitmap\n");
        bitmap_init(app->dup_bitmap);
    }

    last_seq = new_seq;
}

#endif

// ============================================================================
// STEP 7: Common pitfalls to avoid
// ============================================================================

/*
** MISTAKE 1: Testing bit AFTER setting it
** ----------------------------------------
** WRONG:
**   bitmap_set(bitmap, seq);
**   if (bitmap_test(bitmap, seq)) { ... }  // Will ALWAYS be true!
**
** CORRECT:
**   if (bitmap_test(bitmap, seq)) { return; }  // Check first
**   bitmap_set(bitmap, seq);                   // Then set
**
**
** MISTAKE 2: Incrementing stats for duplicates
** ---------------------------------------------
** WRONG:
**   app->rcv_packets++;  // Before checking for duplicate
**
** CORRECT:
**   if (is_duplicate) { return; }
**   app->rcv_packets++;  // Only count non-duplicates
**
**
** MISTAKE 3: Using wrong bit operations
** --------------------------------------
** WRONG:
**   bitmap[idx] = (1 << offset);        // Overwrites entire byte!
**
** CORRECT:
**   bitmap[idx] |= (1 << offset);       // Sets one bit, preserves others
**
**
** MISTAKE 4: Off-by-one in bitmap size
** -------------------------------------
** WRONG:
**   uint8_t bitmap[8191];  // Too small! Missing last byte
**
** CORRECT:
**   uint8_t bitmap[8192];  // 65536 / 8 = 8192 exactly
*/

// ============================================================================
// MAIN: Run all tests
// ============================================================================

int main(void)
{
    test_duplicate_detection();
    return 0;
}

/*
** SUMMARY - Integration Checklist
** ================================
**
** [ ] 1. Add uint8_t dup_bitmap[8192] to t_app structure (ft_ping.h)
** [ ] 2. Add bitmap functions to utils.c or new bitmap.c file
** [ ] 3. Call bitmap_init() in main() or init_app()
** [ ] 4. Modify ping_success() to check bitmap_test() BEFORE processing
** [ ] 5. Call bitmap_set() after confirming packet is not duplicate
** [ ] 6. Print "(DUP!)" suffix when duplicate detected
** [ ] 7. DO NOT increment rcv_packets for duplicates
** [ ] 8. Test with: ping -c 5 <host>, then check for no duplicates
** [ ] 9. Test duplicates with: sudo tcpreplay or packet injection
**
** Memory cost: 8 KB (vs 64 KB for char array)
** Performance: O(1) for all operations (set/test/clear)
**
** Questions to consider:
** - Should duplicates update RTT statistics? (Usually NO)
** - Should duplicates be counted in packet loss %? (Usually NO)
** - Should you clear bitmap on sequence wrap? (Usually YES if long-running)
*/
