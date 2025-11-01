#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Bitmap size: 65536 sequence numbers / 8 bits per byte = 8192 bytes
#define BITMAP_SIZE 8192

// SET a bit: mark sequence number as received
void set_bit(uint8_t *bitmap, uint16_t seq)
{
    uint16_t byte_index = seq >> 3;     // seq / 8
    uint8_t  bit_offset = seq & 7;      // seq % 8
    uint8_t  mask = 1 << bit_offset;

    bitmap[byte_index] |= mask;
}

// TEST a bit: check if sequence number was received
int test_bit(uint8_t *bitmap, uint16_t seq)
{
    uint16_t byte_index = seq >> 3;
    uint8_t  bit_offset = seq & 7;
    uint8_t  mask = 1 << bit_offset;

    return (bitmap[byte_index] & mask) != 0;
}

// CLEAR a bit: reset sequence number
void clear_bit(uint8_t *bitmap, uint16_t seq)
{
    uint16_t byte_index = seq >> 3;
    uint8_t  bit_offset = seq & 7;
    uint8_t  mask = 1 << bit_offset;

    bitmap[byte_index] &= ~mask;
}

// HELPER: Print a byte in binary format
void print_byte_binary(uint8_t byte)
{
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

// DEMO: Visualize bitmap operations
void demo_basic_operations(void)
{
    uint8_t bitmap[BITMAP_SIZE];
    memset(bitmap, 0, BITMAP_SIZE);  // Initialize all bits to 0

    printf("=== BITMAP BASICS ===\n\n");

    // Show initial state
    printf("Initial state of byte 5: ");
    print_byte_binary(bitmap[5]);
    printf("\n\n");

    // Set sequence 42 (byte 5, bit 2)
    printf("Setting bit for sequence 42...\n");
    set_bit(bitmap, 42);
    printf("Byte 5 after setting seq 42: ");
    print_byte_binary(bitmap[5]);
    printf("  <- bit 2 is now ON\n\n");

    // Set sequence 45 (byte 5, bit 5)
    printf("Setting bit for sequence 45...\n");
    set_bit(bitmap, 45);
    printf("Byte 5 after setting seq 45: ");
    print_byte_binary(bitmap[5]);
    printf("  <- bit 5 is also ON\n\n");

    // Test some sequences
    printf("Testing sequences:\n");
    printf("  Seq 42: %s\n", test_bit(bitmap, 42) ? "RECEIVED (duplicate!)" : "Not received");
    printf("  Seq 43: %s\n", test_bit(bitmap, 43) ? "RECEIVED (duplicate!)" : "Not received");
    printf("  Seq 45: %s\n", test_bit(bitmap, 45) ? "RECEIVED (duplicate!)" : "Not received");
    printf("\n");

    // Clear sequence 42
    printf("Clearing bit for sequence 42...\n");
    clear_bit(bitmap, 42);
    printf("Byte 5 after clearing seq 42: ");
    print_byte_binary(bitmap[5]);
    printf("  <- bit 2 is now OFF\n\n");

    printf("Testing seq 42 again: %s\n\n", test_bit(bitmap, 42) ? "RECEIVED" : "Not received");
}

// DEMO: Simulate ping duplicate detection
void demo_ping_duplicates(void)
{
    uint8_t bitmap[BITMAP_SIZE];
    memset(bitmap, 0, BITMAP_SIZE);

    printf("=== PING DUPLICATE DETECTION SIMULATION ===\n\n");

    // Simulate receiving packets
    uint16_t received_sequence[] = {1, 2, 3, 2, 4, 5, 3, 6};  // Note: 2 and 3 are duplicates
    int num_packets = sizeof(received_sequence) / sizeof(received_sequence[0]);

    for (int i = 0; i < num_packets; i++) {
        uint16_t seq = received_sequence[i];

        if (test_bit(bitmap, seq)) {
            printf("Received seq %u: DUPLICATE (already received)\n", seq);
        } else {
            printf("Received seq %u: New packet\n", seq);
            set_bit(bitmap, seq);
        }
    }

    printf("\nFinal state - which sequences were received?\n");
    for (uint16_t seq = 1; seq <= 6; seq++) {
        printf("  Seq %u: %s\n", seq, test_bit(bitmap, seq) ? "YES" : "NO");
    }
}

// DEMO: Edge cases
void demo_edge_cases(void)
{
    uint8_t bitmap[BITMAP_SIZE];
    memset(bitmap, 0, BITMAP_SIZE);

    printf("\n=== EDGE CASES ===\n\n");

    // Test boundaries
    uint16_t edge_values[] = {0, 1, 7, 8, 255, 256, 65535};

    for (int i = 0; i < 7; i++) {
        uint16_t seq = edge_values[i];
        uint16_t byte_idx = seq >> 3;
        uint8_t bit_off = seq & 7;

        set_bit(bitmap, seq);

        printf("Seq %5u: byte[%5u], bit %u - ", seq, byte_idx, bit_off);
        print_byte_binary(bitmap[byte_idx]);
        printf("\n");

        // Clear for next test
        memset(bitmap, 0, BITMAP_SIZE);
    }
}

// DEMO: Performance insight
void demo_performance(void)
{
    printf("\n=== PERFORMANCE INSIGHT ===\n\n");

    printf("Memory usage comparison:\n");
    printf("  Using char array (1 byte per flag):  65,536 bytes (64.0 KB)\n");
    printf("  Using bitmap (1 bit per flag):        8,192 bytes ( 8.0 KB)\n");
    printf("  Space saved:                          87.5%%\n\n");

    printf("Why shift/AND instead of division/modulo?\n");
    printf("  seq / 8:   Compiler may use division (slower)\n");
    printf("  seq >> 3:  Guaranteed bit shift (faster)\n");
    printf("  seq %% 8:   Compiler may use modulo (slower)\n");
    printf("  seq & 7:   Guaranteed AND operation (faster)\n\n");

    printf("Math proof that they're equivalent:\n");
    printf("  42 / 8 = 5    vs    42 >> 3 = 5   ✓\n");
    printf("  42 %% 8 = 2    vs    42 & 7  = 2   ✓\n");
}

int main(void)
{
    demo_basic_operations();
    demo_ping_duplicates();
    demo_edge_cases();
    demo_performance();

    return 0;
}
