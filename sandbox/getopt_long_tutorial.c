/* ************************************************************************** */
/*                                                                            */
/*   getopt_long_tutorial.c - Complete tutorial on GNU getopt_long()         */
/*                                                                            */
/*   This demonstrates getopt_long() which supports both short and long      */
/*   options: -v/--verbose, -c 5/--count=5, etc.                             */
/*                                                                            */
/*   Used by: GNU coreutils, git, ssh, curl, and most modern CLI tools       */
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
 * Configuration structure identical to basic getopt example
 */
typedef struct s_config {
    int     verbose;
    int     numeric;
    int     count;
    int     ttl;
    int     quiet;          // New option for long-only example
    char    *interface;     // New option for long-only example
    char    *hostname;
} t_config;

/*
 * Print usage - now includes both short and long options
 */
void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [OPTIONS] hostname\n", prog_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -v, --verbose           Verbose output\n");
    fprintf(stderr, "  -n, --numeric           Numeric output (don't resolve)\n");
    fprintf(stderr, "  -c, --count <num>       Number of packets to send\n");
    fprintf(stderr, "  -t, --ttl <num>         Time to live (1-255)\n");
    fprintf(stderr, "  -q, --quiet             Quiet mode (no output except errors)\n");
    fprintf(stderr, "  -I, --interface <dev>   Use specific interface (eth0, wlan0, etc.)\n");
    fprintf(stderr, "  -h, --help              Show this help message\n");
    fprintf(stderr, "\nLong-option styles:\n");
    fprintf(stderr, "  --count 5               Space-separated\n");
    fprintf(stderr, "  --count=5               Equals sign (preferred for clarity)\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s --verbose --count=5 google.com\n", prog_name);
    fprintf(stderr, "  %s -vc5 --ttl=128 localhost\n", prog_name);
    fprintf(stderr, "  %s --quiet -I eth0 8.8.8.8\n", prog_name);
}

/* Validation helper (same as basic version) */
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
 * Parse arguments using getopt_long()
 *
 * getopt_long() is a GNU extension (but widely available) that adds:
 * - Long options: --verbose instead of just -v
 * - Hybrid support: both -v and --verbose work simultaneously
 * - Two styles: --count 5 or --count=5
 *
 * Returns: 0 on success, 1 on error
 */
int parse_arguments(int argc, char **argv, t_config *config) {
    int opt;
    int option_index = 0;

    /*
     * Long options are defined in a struct option array
     * Each entry has:
     *   name       - long option name (without --)
     *   has_arg    - no_argument(0), required_argument(1), optional_argument(2)
     *   flag       - if NULL, getopt_long returns 'val'; if non-NULL, sets *flag=val
     *   val        - value to return (usually the short option character)
     */
    static struct option long_options[] = {
        /* name,        has_arg,            flag,   val */
        {"verbose",     no_argument,        NULL,   'v'},   // maps to -v
        {"numeric",     no_argument,        NULL,   'n'},   // maps to -n
        {"count",       required_argument,  NULL,   'c'},   // maps to -c
        {"ttl",         required_argument,  NULL,   't'},   // maps to -t
        {"quiet",       no_argument,        NULL,   'q'},   // maps to -q
        {"interface",   required_argument,  NULL,   'I'},   // maps to -I
        {"help",        no_argument,        NULL,   'h'},   // maps to -h
        {0,             0,                  0,      0}      // sentinel (required)
    };

    /*
     * The short option string works exactly like getopt()
     * Even with long options defined, short options still work
     */
    const char *short_options = "vnc:t:qI:h";

    opterr = 0;  /* We handle errors ourselves */

    /*
     * getopt_long() instead of getopt()
     * Parameters:
     *   argc, argv        - same as main()
     *   short_options     - string like getopt() (can be NULL if only long options)
     *   long_options      - array of struct option
     *   &option_index     - output parameter: index in long_options[] (can be NULL)
     */
    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        /*
         * When a long option is matched, getopt_long() returns its 'val' field
         * So --verbose and -v both return 'v'
         * This means our switch statement handles both!
         */

        switch (opt) {
            case 'v':
                /* Triggered by BOTH -v and --verbose */
                config->verbose = 1;
                printf("[DEBUG] Verbose mode enabled\n");
                break;

            case 'n':
                /* Triggered by BOTH -n and --numeric */
                config->numeric = 1;
                printf("[DEBUG] Numeric mode enabled\n");
                break;

            case 'c':
                /* Triggered by -c, -c5, --count 5, --count=5 */
                if (!is_valid_number(optarg)) {
                    fprintf(stderr, "Error: count requires a positive integer\n");
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
                /* Triggered by -t, -t128, --ttl 128, --ttl=128 */
                if (!is_valid_number(optarg)) {
                    fprintf(stderr, "Error: TTL requires a positive integer\n");
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

            case 'q':
                /* New option: quiet mode */
                config->quiet = 1;
                printf("[DEBUG] Quiet mode enabled\n");
                break;

            case 'I':
                /*
                 * Interface name option (string argument)
                 * No special validation needed - kernel will reject invalid interfaces
                 */
                config->interface = optarg;
                printf("[DEBUG] Interface set to: %s\n", config->interface);
                break;

            case 'h':
                /* Help triggered by -h or --help */
                print_usage(argv[0]);
                exit(0);

            case '?':
                /*
                 * Error handling for long options is slightly different:
                 * - For short options: optopt contains the character
                 * - For long options: optopt is 0, use argv[optind-1] for the name
                 */
                if (optopt) {
                    /* Short option error */
                    if (optopt == 'c' || optopt == 't' || optopt == 'I') {
                        fprintf(stderr, "Error: -%c requires an argument\n", optopt);
                    } else if (isprint(optopt)) {
                        fprintf(stderr, "Error: unknown option '-%c'\n", optopt);
                    } else {
                        fprintf(stderr, "Error: unknown option character '\\x%x'\n", optopt);
                    }
                } else {
                    /* Long option error */
                    fprintf(stderr, "Error: unknown option '%s'\n", argv[optind - 1]);
                }
                print_usage(argv[0]);
                return 1;

            default:
                fprintf(stderr, "Error: getopt_long returned unexpected value: %d\n", opt);
                return 1;
        }
    }

    /* Same positional argument handling as basic getopt */
    if (optind >= argc) {
        fprintf(stderr, "Error: missing hostname\n");
        print_usage(argv[0]);
        return 1;
    }

    config->hostname = argv[optind];

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
 * Main function
 */
int main(int argc, char **argv) {
    t_config config = {0};

    /* Set defaults */
    config.count = 4;
    config.ttl = 64;

    printf("=== getopt_long() Tutorial: Long Options ===\n\n");

    printf("Command line:\n  ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n\n");

    if (parse_arguments(argc, argv, &config) != 0) {
        return 1;
    }

    printf("\n=== Parsed Configuration ===\n");
    printf("Hostname:  %s\n", config.hostname);
    printf("Verbose:   %s\n", config.verbose ? "YES" : "NO");
    printf("Numeric:   %s\n", config.numeric ? "YES" : "NO");
    printf("Quiet:     %s\n", config.quiet ? "YES" : "NO");
    printf("Count:     %d packets\n", config.count);
    printf("TTL:       %d\n", config.ttl);
    printf("Interface: %s\n", config.interface ? config.interface : "(default)");

    printf("\n=== Simulated Execution ===\n");
    if (!config.quiet) {
        printf("PING %s with %d packets", config.hostname, config.count);
        if (config.verbose)
            printf(" (verbose)");
        if (config.numeric)
            printf(" (numeric)");
        printf("\n");
        printf("Using TTL: %d\n", config.ttl);
        if (config.interface)
            printf("Using interface: %s\n", config.interface);
    } else {
        printf("(quiet mode - minimal output)\n");
    }

    return 0;
}

/*
 * COMPILATION AND TESTING:
 *
 * Compile:
 *   gcc -Wall -Wextra -o getopt_long_tutorial getopt_long_tutorial.c
 *
 * Test long options with =:
 *   ./getopt_long_tutorial --verbose --count=5 localhost
 *   ./getopt_long_tutorial --ttl=128 --count=10 8.8.8.8
 *
 * Test long options with space:
 *   ./getopt_long_tutorial --verbose --count 5 localhost
 *   ./getopt_long_tutorial --ttl 128 --count 10 8.8.8.8
 *
 * Test short options (still work!):
 *   ./getopt_long_tutorial -v -c 5 localhost
 *   ./getopt_long_tutorial -vc5 -t128 google.com
 *
 * Test mixing short and long:
 *   ./getopt_long_tutorial -v --count=5 localhost
 *   ./getopt_long_tutorial --verbose -c5 --ttl=64 8.8.8.8
 *
 * Test new options:
 *   ./getopt_long_tutorial --quiet --interface=eth0 127.0.0.1
 *   ./getopt_long_tutorial -q -I wlan0 google.com
 *
 * Test help:
 *   ./getopt_long_tutorial --help
 *   ./getopt_long_tutorial -h
 *
 * Test error cases:
 *   ./getopt_long_tutorial --count              # Missing argument
 *   ./getopt_long_tutorial --unknown localhost  # Unknown option
 *   ./getopt_long_tutorial --count=abc host     # Invalid number
 */

/*
 * ADVANCED: Optional Arguments
 *
 * Some options take optional arguments (rare, but useful for things like
 * debug levels: -d vs -d3)
 *
 * Example:
 *   {"debug", optional_argument, NULL, 'd'}
 *
 * With optional_argument:
 *   -d           optarg is NULL
 *   -d3          optarg is "3"
 *   -d 3         optarg is NULL (the 3 becomes positional!)
 *   --debug      optarg is NULL
 *   --debug=3    optarg is "3"
 *
 * PITFALL: For optional arguments, MUST use = with long options!
 * --debug 3 treats 3 as positional, NOT as the debug value
 */

/*
 * ADVANCED: Flag Variables
 *
 * If you set the 'flag' field to non-NULL, getopt_long() will:
 * 1. Set *flag = val
 * 2. Return 0 (not val)
 *
 * Example:
 *   int verbose_flag = 0;
 *   {"verbose", no_argument, &verbose_flag, 1}
 *
 * Now when --verbose is used, verbose_flag is automatically set to 1,
 * and getopt_long() returns 0 (so you don't need a case statement)
 *
 * This is useful for simple flags, but makes debugging harder and
 * prevents adding validation logic, so most code doesn't use it.
 */

/*
 * KEY TAKEAWAYS:
 *
 * 1. getopt_long() is backwards compatible with getopt()
 * 2. Short and long options can coexist and map to the same 'val'
 * 3. Long options support both --opt=arg and --opt arg syntax
 * 4. The struct option array must end with a {0,0,0,0} sentinel
 * 5. Long options make CLIs more user-friendly (self-documenting)
 * 6. Short options are faster to type for power users
 * 7. Supporting both is best practice for production tools
 *
 * WHEN TO USE getopt() vs getopt_long():
 *
 * Use getopt():
 *   - Simple tools with few options
 *   - 42 school projects (if getopt_long not allowed)
 *   - Maximum portability (POSIX-only systems)
 *
 * Use getopt_long():
 *   - Professional/production tools
 *   - Many options (long names are clearer)
 *   - User-facing applications (better UX)
 *   - When you want GNU-style --option=value syntax
 */
