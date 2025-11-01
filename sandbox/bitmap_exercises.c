/*
** BITMAP EXERCISES - Practice Bit Manipulation
** ==============================================
**
** Work through these exercises to solidify your understanding.
** Each exercise has a TODO section where you write the code.
** Run the program to check your answers.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// ============================================================================
// EXERCISE 1: Manual Binary Conversion
// ============================================================================

/*
** Convert these decimal numbers to binary (8-bit):
**
** 5  = 00000101    (check: 4 + 1 = 5)
** 42 = 00101010    (check: 32 + 8 + 2 = 42)
** 128 = 10000000   (check: 128)
** 255 = 11111111   (check: 128+64+32+16+8+4+2+1 = 255)
**
** Now verify your answers:
*/

void exercise1_verify(void)
{
    printf("=== EXERCISE 1: Binary Conversion ===\n\n");

    uint8_t values[] = {5, 42, 128, 255};

    for (int i = 0; i < 4; i++) {
        printf("Decimal %3u = Binary ", values[i]);
        for (int bit = 7; bit >= 0; bit--) {
            printf("%d", (values[i] >> bit) & 1);
        }
        printf("\n");
    }
    printf("\n");
}

// ============================================================================
// EXERCISE 2: Understanding Bit Positions
// ============================================================================

/*
** Fill in the blanks:
**
** If we have the number 42 (binary: 00101010):
**
** Bit 1 is: ON (1)
** Bit 3 is: ON (1)
** Bit 5 is: ON (1)
** Bit 0 is: OFF (0)
** Bit 7 is: OFF (0)
**
** Which bit positions are ON? Answer: 1, 3, 5
** Sum of their values: 2 + 8 + 32 = 42 ✓
*/

void exercise2_which_bits_set(void)
{
    printf("=== EXERCISE 2: Which Bits Are Set? ===\n\n");

    uint8_t value = 42;

    printf("Value %u (binary: ", value);
    for (int bit = 7; bit >= 0; bit--) {
        printf("%d", (value >> bit) & 1);
    }
    printf(")\n\n");

    printf("Bits that are ON:\n");
    for (int bit = 0; bit < 8; bit++) {
        if (value & (1 << bit)) {
            printf("  Bit %d (value %d)\n", bit, 1 << bit);
        }
    }
    printf("\n");
}

// ============================================================================
// EXERCISE 3: Create a Mask
// ============================================================================

/*
** TODO: Complete this function
**
** Task: Create a mask with ONLY bit N set to 1, all others 0
**
** Examples:
**   create_mask(0) should return 00000001 (decimal 1)
**   create_mask(3) should return 00001000 (decimal 8)
**   create_mask(7) should return 10000000 (decimal 128)
**
** Hint: Use the left shift operator (<<)
*/

uint8_t create_mask(uint8_t bit_position)
{
    // TODO: Replace this return statement with your code
    return 1 << bit_position;
}

void exercise3_test_masks(void)
{
    printf("=== EXERCISE 3: Creating Masks ===\n\n");

    for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t mask = create_mask(bit);

        printf("Bit %u mask: ", bit);
        for (int i = 7; i >= 0; i--) {
            printf("%d", (mask >> i) & 1);
        }
        printf(" (decimal %3u)\n", mask);
    }
    printf("\n");
}

// ============================================================================
// EXERCISE 4: Test if Bit is Set
// ============================================================================

/*
** TODO: Complete this function
**
** Task: Return 1 if bit N is set in value, 0 otherwise
**
** Examples:
**   is_bit_set(42, 1) should return 1  (bit 1 is ON in 00101010)
**   is_bit_set(42, 0) should return 0  (bit 0 is OFF in 00101010)
**   is_bit_set(42, 5) should return 1  (bit 5 is ON in 00101010)
**
** Hint: Create a mask, then use AND (&) to test
*/

int is_bit_set(uint8_t value, uint8_t bit_position)
{
    // TODO: Replace this with your code
    uint8_t mask = 1 << bit_position;
    return (value & mask) != 0;
}

void exercise4_test_bit_checking(void)
{
    printf("=== EXERCISE 4: Testing Bits ===\n\n");

    uint8_t value = 42;  // Binary: 00101010

    printf("Testing value %u (binary: 00101010):\n\n", value);

    for (uint8_t bit = 0; bit < 8; bit++) {
        int is_set = is_bit_set(value, bit);
        printf("  Bit %u: %s\n", bit, is_set ? "SET" : "NOT SET");
    }
    printf("\n");
}

// ============================================================================
// EXERCISE 5: Set a Bit
// ============================================================================

/*
** TODO: Complete this function
**
** Task: Turn ON bit N in value (without affecting other bits)
**
** Examples:
**   set_bit(0, 0) should return 00000001 (decimal 1)
**   set_bit(0, 5) should return 00100000 (decimal 32)
**   set_bit(42, 0) should return 00101011 (decimal 43) - turned ON bit 0
**   set_bit(42, 1) should return 00101010 (decimal 42) - already ON, no change
**
** Hint: Use OR (|) to set bits without affecting others
*/

uint8_t set_bit(uint8_t value, uint8_t bit_position)
{
    // TODO: Replace this with your code
    return value | (1 << bit_position);
}

void exercise5_test_setting_bits(void)
{
    printf("=== EXERCISE 5: Setting Bits ===\n\n");

    uint8_t value = 0;  // Start with all bits OFF

    printf("Starting value: 00000000\n\n");

    // Set bits 1, 3, and 5
    value = set_bit(value, 1);
    printf("After setting bit 1: ");
    for (int i = 7; i >= 0; i--) printf("%d", (value >> i) & 1);
    printf(" (decimal %u)\n", value);

    value = set_bit(value, 3);
    printf("After setting bit 3: ");
    for (int i = 7; i >= 0; i--) printf("%d", (value >> i) & 1);
    printf(" (decimal %u)\n", value);

    value = set_bit(value, 5);
    printf("After setting bit 5: ");
    for (int i = 7; i >= 0; i--) printf("%d", (value >> i) & 1);
    printf(" (decimal %u)\n", value);

    printf("\nFinal value should be 00101010 (decimal 42)\n");
    printf("Success: %s\n\n", value == 42 ? "YES ✓" : "NO ✗");
}

// ============================================================================
// EXERCISE 6: Clear a Bit
// ============================================================================

/*
** TODO: Complete this function
**
** Task: Turn OFF bit N in value (without affecting other bits)
**
** Examples:
**   clear_bit(255, 0) should return 11111110 (decimal 254)
**   clear_bit(42, 1) should return 00101000 (decimal 40)
**   clear_bit(42, 0) should return 00101010 (decimal 42) - already OFF
**
** Hint: Create mask, invert it with NOT (~), then AND (&)
*/

uint8_t clear_bit(uint8_t value, uint8_t bit_position)
{
    // TODO: Replace this with your code
    return value & ~(1 << bit_position);
}

void exercise6_test_clearing_bits(void)
{
    printf("=== EXERCISE 6: Clearing Bits ===\n\n");

    uint8_t value = 255;  // All bits ON: 11111111

    printf("Starting value: 11111111 (decimal 255)\n\n");

    // Clear bits 0, 3, 7
    value = clear_bit(value, 0);
    printf("After clearing bit 0: ");
    for (int i = 7; i >= 0; i--) printf("%d", (value >> i) & 1);
    printf(" (decimal %u)\n", value);

    value = clear_bit(value, 3);
    printf("After clearing bit 3: ");
    for (int i = 7; i >= 0; i--) printf("%d", (value >> i) & 1);
    printf(" (decimal %u)\n", value);

    value = clear_bit(value, 7);
    printf("After clearing bit 7: ");
    for (int i = 7; i >= 0; i--) printf("%d", (value >> i) & 1);
    printf(" (decimal %u)\n", value);

    printf("\nFinal value should be 01110110 (decimal 118)\n");
    printf("Success: %s\n\n", value == 118 ? "YES ✓" : "NO ✗");
}

// ============================================================================
// EXERCISE 7: Calculate Bitmap Position
// ============================================================================

/*
** TODO: Complete these functions
**
** Task: For a given sequence number, calculate:
**   - Which byte index in the bitmap array
**   - Which bit offset within that byte
**
** Examples:
**   Sequence 0:  byte_index=0,  bit_offset=0
**   Sequence 7:  byte_index=0,  bit_offset=7
**   Sequence 8:  byte_index=1,  bit_offset=0
**   Sequence 42: byte_index=5,  bit_offset=2
**
** Hint: byte_index = seq / 8 (or seq >> 3)
**       bit_offset = seq % 8 (or seq & 7)
*/

uint16_t get_byte_index(uint16_t seq)
{
    // TODO: Replace this with your code
    return seq >> 3;  // or: seq / 8
}

uint8_t get_bit_offset(uint16_t seq)
{
    // TODO: Replace this with your code
    return seq & 7;   // or: seq % 8
}

void exercise7_test_positions(void)
{
    printf("=== EXERCISE 7: Bitmap Position Calculation ===\n\n");

    uint16_t test_sequences[] = {0, 7, 8, 42, 255, 256, 1000, 65535};

    printf("Seq    | Byte Index | Bit Offset | Verification\n");
    printf("-------|------------|------------|--------------\n");

    for (int i = 0; i < 8; i++) {
        uint16_t seq = test_sequences[i];
        uint16_t byte_idx = get_byte_index(seq);
        uint8_t bit_off = get_bit_offset(seq);

        // Verify: (byte_idx * 8) + bit_off should equal seq
        int correct = ((byte_idx * 8) + bit_off) == seq;

        printf("%6u | %10u | %10u | %s\n",
               seq, byte_idx, bit_off, correct ? "✓" : "✗ WRONG!");
    }
    printf("\n");
}

// ============================================================================
// EXERCISE 8: Complete Bitmap Implementation
// ============================================================================

/*
** TODO: Implement a complete bitmap for tracking duplicates
**
** You'll implement three functions using what you learned:
**   1. bitmap_mark_received() - Mark sequence as received
**   2. bitmap_check_duplicate() - Check if already received
**   3. bitmap_reset() - Clear all bits
*/

#define MAX_SEQ 100  // Simplified: tracking 0-99 (needs 13 bytes)

typedef struct {
    uint8_t bits[13];  // 100 sequences / 8 bits per byte = 12.5, round up to 13
} t_bitmap;

void bitmap_mark_received(t_bitmap *bm, uint16_t seq)
{
    // TODO: Implement this
    uint16_t byte_idx = seq >> 3;
    uint8_t bit_off = seq & 7;
    bm->bits[byte_idx] |= (1 << bit_off);
}

int bitmap_check_duplicate(t_bitmap *bm, uint16_t seq)
{
    // TODO: Implement this
    uint16_t byte_idx = seq >> 3;
    uint8_t bit_off = seq & 7;
    return (bm->bits[byte_idx] & (1 << bit_off)) != 0;
}

void bitmap_reset(t_bitmap *bm)
{
    // TODO: Implement this
    memset(bm->bits, 0, sizeof(bm->bits));
}

void exercise8_full_implementation(void)
{
    printf("=== EXERCISE 8: Complete Bitmap Implementation ===\n\n");

    t_bitmap bm;
    bitmap_reset(&bm);

    // Simulate receiving packets
    uint16_t received[] = {1, 2, 5, 2, 10, 5, 99, 50, 1};

    printf("Simulating packet reception:\n\n");

    for (int i = 0; i < 9; i++) {
        uint16_t seq = received[i];

        if (bitmap_check_duplicate(&bm, seq)) {
            printf("Seq %2u: DUPLICATE (already received)\n", seq);
        } else {
            printf("Seq %2u: NEW packet\n", seq);
            bitmap_mark_received(&bm, seq);
        }
    }

    printf("\n");
}

// ============================================================================
// MAIN: Run all exercises
// ============================================================================

int main(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         BITMAP MASTERY: Hands-On Exercises                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    exercise1_verify();
    exercise2_which_bits_set();
    exercise3_test_masks();
    exercise4_test_bit_checking();
    exercise5_test_setting_bits();
    exercise6_test_clearing_bits();
    exercise7_test_positions();
    exercise8_full_implementation();

    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Congratulations! You've mastered bitmap fundamentals.    ║\n");
    printf("║  You're now ready to implement duplicate detection in     ║\n");
    printf("║  your ft_ping project!                                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    return 0;
}
