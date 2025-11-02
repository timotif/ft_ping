/* ************************************************************************** */
/*                                                                            */
/*   getopt_tutorial.c - Complete tutorial on POSIX getopt()                 */
/*                                                                            */
/*   This demonstrates the industry-standard approach to argument parsing    */
/*   used in virtually all Unix/Linux command-line tools.                    */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/*
 * Configuration structure to hold parsed arguments
 * This is a common pattern - parse into a struct, then use it throughout
 */
typedef struct s_config {
    int     verbose;        // -v flag
    int     numeric;        // -n flag
    int     count;          // -c <num>
    int     ttl;            // -t <num>
    char    *hostname;      // positional argument
} t_config;

/*
 * Print usage information
 * Convention: prog_name comes from argv[0]
 */
void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [OPTIONS] hostname\n", prog_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -v          Verbose output\n");
    fprintf(stderr, "  -n          Numeric output (don't resolve addresses)\n");
    fprintf(stderr, "  -c <count>  Number of packets to send\n");
    fprintf(stderr, "  -t <ttl>    Time to live\n");
    fprintf(stderr, "  -h          Show this help message\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s 8.8.8.8\n", prog_name);
    fprintf(stderr, "  %s -v -c 5 google.com\n", prog_name);
    fprintf(stderr, "  %s -vc5 -t64 localhost\n", prog_name);
    fprintf(stderr, "  %s -c 10 -v -n 127.0.0.1\n", prog_name);
}

/*
 * Validate that a string contains only digits (for -c and -t arguments)
 * Returns: 1 if valid number, 0 otherwise
 */
int is_valid_number(const char *str) {
    if (!str || !*str)
        return 0;

    while (*str) {
        if (!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

/*
 * Parse command-line arguments using getopt()
 *
 * getopt() is the POSIX standard for argument parsing. It:
 * - Handles short options (-v, -c, etc.)
 * - Supports option clustering (-vc is same as -v -c)
 * - Handles options with arguments (-c 5 or -c5)
 * - Automatically sets optarg/optind/opterr/optopt
 *
 * The optstring "vnc:t:h" means:
 *   v, n, h  - flags with no arguments
 *   c:, t:   - options that require an argument (: means "requires arg")
 *
 * Returns: 0 on success, 1 on error
 */
int parse_arguments(int argc, char **argv, t_config *config) {
    int opt;

    /*
     * getopt() global variables (declared in <unistd.h>):
     * - optarg: points to the argument of the current option (e.g., "5" in "-c 5")
     * - optind: index of next element to process in argv (used after getopt loop)
     * - opterr: if non-zero, getopt prints error messages (we usually set to 0)
     * - optopt: the actual option character when getopt returns '?'
     */

    /* Disable getopt's built-in error messages - we'll handle errors ourselves */
    opterr = 0;

    /*
     * The getopt() loop:
     * - Returns the next option character each iteration
     * - Returns -1 when all options have been processed
     * - Returns '?' for unrecognized options or missing required arguments
     * - Returns ':' if the first character of optstring is ':' and arg is missing
     */
    while ((opt = getopt(argc, argv, "vnc:t:h")) != -1) {
        switch (opt) {
            case 'v':
                /* Simple flag - just set it */
                config->verbose = 1;
                printf("[DEBUG] Verbose mode enabled\n");
                break;

            case 'n':
                /* Another simple flag */
                config->numeric = 1;
                printf("[DEBUG] Numeric mode enabled\n");
                break;

            case 'c':
                /*
                 * Option with required argument
                 * optarg points to the argument string ("5" in "-c 5" or "-c5")
                 * Always validate before using!
                 */
                if (!is_valid_number(optarg)) {
                    fprintf(stderr, "Error: -c requires a positive integer\n");
                    fprintf(stderr, "Got: '%s'\n", optarg);
                    return 1;
                }
                config->count = atoi(optarg);
                if (config->count <= 0 || config->count > 10000) {
                    fprintf(stderr, "Error: count must be between 1 and 10000\n");
                    return 1;
                }
                printf("[DEBUG] Count set to: %d\n", config->count);
                break;

            case 't':
                /* Same pattern as -c: validate, then parse */
                if (!is_valid_number(optarg)) {
                    fprintf(stderr, "Error: -t requires a positive integer\n");
                    fprintf(stderr, "Got: '%s'\n", optarg);
                    return 1;
                }
                config->ttl = atoi(optarg);
                if (config->ttl <= 0 || config->ttl > 255) {
                    fprintf(stderr, "Error: TTL must be between 1 and 255\n");
                    return 1;
                }
                printf("[DEBUG] TTL set to: %d\n", config->ttl);
                break;

            case 'h':
                /* Help requested - print usage and exit successfully */
                print_usage(argv[0]);
                exit(0);

            case '?':
                /*
                 * Unknown option or missing required argument
                 * optopt contains the problematic option character
                 */
                if (optopt == 'c' || optopt == 't') {
                    fprintf(stderr, "Error: -%c requires an argument\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Error: unknown option '-%c'\n", optopt);
                } else {
                    fprintf(stderr, "Error: unknown option character '\\x%x'\n", optopt);
                }
                print_usage(argv[0]);
                return 1;

            default:
                /* Should never reach here */
                fprintf(stderr, "Error: getopt returned unexpected value: %d\n", opt);
                return 1;
        }
    }

    /*
     * After getopt() finishes, optind points to the first non-option argument
     * This is where positional arguments start (hostname in our case)
     */
    printf("[DEBUG] optind=%d, argc=%d\n", optind, argc);

    if (optind >= argc) {
        /* No hostname provided */
        fprintf(stderr, "Error: missing hostname\n");
        print_usage(argv[0]);
        return 1;
    }

    /* First positional argument is the hostname */
    config->hostname = argv[optind];

    /* Check for extra arguments (we only expect one positional argument) */
    if (optind + 1 < argc) {
        fprintf(stderr, "Warning: extra arguments ignored:");
        for (int i = optind + 1; i < argc; i++) {
            fprintf(stderr, " %s", argv[i]);
        }
        fprintf(stderr, "\n");
    }

    return 0;
}

/*
 * Main function demonstrating complete argument parsing workflow
 */
int main(int argc, char **argv) {
    t_config config = {0};  /* Zero-initialize - all flags start as 0/NULL */

    /* Set default values for options */
    config.count = 4;       /* Default: send 4 packets */
    config.ttl = 64;        /* Default TTL */

    printf("=== getopt() Tutorial: Parsing Arguments ===\n\n");

    /* Show what we received */
    printf("Command line:\n  ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n\n");

    /* Parse arguments */
    if (parse_arguments(argc, argv, &config) != 0) {
        return 1;
    }

    /* Display parsed configuration */
    printf("\n=== Parsed Configuration ===\n");
    printf("Hostname: %s\n", config.hostname);
    printf("Verbose:  %s\n", config.verbose ? "YES" : "NO");
    printf("Numeric:  %s\n", config.numeric ? "YES" : "NO");
    printf("Count:    %d packets\n", config.count);
    printf("TTL:      %d\n", config.ttl);

    /* Simulate what a real program would do */
    printf("\n=== Simulated Execution ===\n");
    printf("PING %s with %d packets", config.hostname, config.count);
    if (config.verbose) {
        printf(" (verbose mode)");
    }
    if (config.numeric) {
        printf(" (numeric mode)");
    }
    printf("\n");
    printf("Using TTL: %d\n", config.ttl);

    return 0;
}

/*
 * COMPILATION AND TESTING:
 *
 * Compile:
 *   gcc -Wall -Wextra -o getopt_tutorial getopt_tutorial.c
 *
 * Test basic usage:
 *   ./getopt_tutorial 8.8.8.8
 *   ./getopt_tutorial -v localhost
 *
 * Test option clustering:
 *   ./getopt_tutorial -vn 127.0.0.1
 *   ./getopt_tutorial -vc5 google.com
 *
 * Test options with arguments (both styles work):
 *   ./getopt_tutorial -c 10 8.8.8.8
 *   ./getopt_tutorial -c10 8.8.8.8
 *   ./getopt_tutorial -c 10 -t 128 example.com
 *   ./getopt_tutorial -c10 -t128 example.com
 *
 * Test combined:
 *   ./getopt_tutorial -vc5 -t128 localhost
 *
 * Test error cases:
 *   ./getopt_tutorial                    # Missing hostname
 *   ./getopt_tutorial -x localhost       # Unknown option
 *   ./getopt_tutorial -c                 # Missing argument
 *   ./getopt_tutorial -c abc localhost   # Invalid number
 *   ./getopt_tutorial -c 0 localhost     # Out of range
 *
 * Test help:
 *   ./getopt_tutorial -h
 */

/*
 * KEY TAKEAWAYS:
 *
 * 1. getopt() is the POSIX standard - it's in every Unix/Linux system
 * 2. It handles all the tedious parsing: clustering, arguments, etc.
 * 3. Always validate optarg before using it (could be malformed)
 * 4. Use optind to access positional arguments after parsing
 * 5. Set opterr=0 and handle errors yourself for better messages
 * 6. Parse into a config struct for clean separation of concerns
 * 7. Set defaults before parsing, then override with options
 *
 * COMMON PITFALLS TO AVOID:
 *
 * 1. DON'T use atoi() without validation (it silently returns 0 on error)
 * 2. DON'T forget to check if optind < argc (missing positional args)
 * 3. DON'T modify argv[] - getopt() does internal pointer arithmetic
 * 4. DON'T forget the ':' after options that need arguments ("c:" not "c")
 * 5. DON'T mix getopt() calls across files (uses global state)
 */
