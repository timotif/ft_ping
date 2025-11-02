# Complete Guide to Argument Parsing in C

**Purpose**: This guide teaches you the industry-standard approach to command-line argument parsing used in virtually all Unix/Linux tools. Stop reinventing the wheel.

---

## Table of Contents

1. [Why getopt()?](#why-getopt)
2. [How getopt() Works](#how-getopt-works)
3. [Complete getopt() Reference](#complete-getopt-reference)
4. [Common Patterns](#common-patterns)
5. [Error Handling Best Practices](#error-handling-best-practices)
6. [getopt() vs getopt_long()](#getopt-vs-getopt_long)
7. [Real-World Examples](#real-world-examples)
8. [Common Pitfalls](#common-pitfalls)
9. [Advanced Techniques](#advanced-techniques)

---

## Why getopt()?

### The Problem

Manual argument parsing is error-prone and tedious:

```c
// BAD: Manual parsing (what you might be doing now)
for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0) {
        verbose = 1;
    } else if (strcmp(argv[i], "-c") == 0) {
        i++;  // BUG: What if there's no next argument?
        count = atoi(argv[i]);  // BUG: atoi returns 0 on error
    } else if (argv[i][0] == '-') {
        // How do you handle -vc5? -v -c5? -c 5?
    }
}
```

Problems with manual parsing:
- Doesn't handle option clustering (`-vc` for `-v -c`)
- Doesn't handle attached arguments (`-c5` vs `-c 5`)
- Error handling is complex and repetitive
- Doesn't follow POSIX conventions
- Reinventing the wheel

### The Solution: getopt()

**getopt() is the POSIX standard** for parsing command-line options. It's:
- Available on every Unix/Linux system (including macOS)
- Used by virtually all command-line tools (ls, ping, curl, git, etc.)
- Battle-tested for 40+ years
- Handles all the edge cases for you

**Example tools using getopt/getopt_long:**
- GNU coreutils (ls, cat, grep, etc.)
- inetutils (ping, traceroute, ftp)
- git, ssh, curl, gcc, make
- Basically every command-line tool you've ever used

---

## How getopt() Works

### The Algorithm

getopt() iterates through `argv[]` and:
1. Identifies options (strings starting with `-`)
2. Separates options from their arguments
3. Handles option clustering (`-vn` → `-v -n`)
4. Handles attached arguments (`-c5` → `-c` with arg `5`)
5. Sets global variables for you to use
6. Returns the next option character each call
7. Returns -1 when done

### Global Variables

getopt() uses these global variables (declared in `<unistd.h>`):

```c
extern char *optarg;  // Pointer to current option's argument
extern int optind;    // Index of next argv element to process
extern int opterr;    // If non-zero, print error messages
extern int optopt;    // The actual option character when error occurs
```

**optarg**: Points to the argument string
```c
// For: -c 5  or  -c5
optarg = "5"
```

**optind**: Index in argv[] of the next element to process
```c
// Before parsing: optind = 1
// After parsing all options: optind = index of first positional arg
```

**opterr**: Controls automatic error messages
```c
opterr = 0;  // Disable built-in errors (you handle them)
opterr = 1;  // Enable built-in errors (default)
```

**optopt**: The option character when getopt() returns '?'
```c
case '?':
    if (optopt == 'c')
        fprintf(stderr, "Option -%c requires an argument\n", optopt);
```

### The optstring Specification

The second argument to getopt() is the "option string" that specifies valid options:

```c
getopt(argc, argv, "vnc:t:h")
```

**Syntax:**
- `v` - flag with no argument
- `c:` - option that requires an argument (the `:` means "requires arg")
- `::` - option with optional argument (GNU extension, rarely used)

**Examples:**
```c
"abc"       // -a, -b, -c (all flags, no arguments)
"a:b:c"     // -a <arg>, -b <arg>, -c (first two need arguments)
"vhc:t:"    // -v, -h (flags), -c <arg>, -t <arg>
":abc"      // Special: leading ':' changes error handling (returns ':' for missing arg)
```

### Return Values

getopt() returns:
- The option character (`'v'`, `'c'`, etc.) when an option is found
- `-1` when all options have been processed
- `'?'` when an unknown option or missing argument is encountered

---

## Complete getopt() Reference

### Basic Template

```c
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int opt;
    int verbose = 0;
    int count = 0;
    char *filename = NULL;

    // Disable automatic error messages
    opterr = 0;

    // Parse options
    while ((opt = getopt(argc, argv, "vc:f:")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            case 'c':
                count = atoi(optarg);  // TODO: validate!
                break;
            case 'f':
                filename = optarg;
                break;
            case '?':
                // Error handling
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                return 1;
        }
    }

    // Process positional arguments
    for (int i = optind; i < argc; i++) {
        printf("Positional arg: %s\n", argv[i]);
    }

    return 0;
}
```

### What getopt() Handles Automatically

**Option clustering:**
```bash
-vn          # Parsed as: -v, -n
-vc5         # Parsed as: -v, -c with arg "5"
-c5vn        # Parsed as: -c with arg "5vn" (first option wins)
```

**Argument styles (both work):**
```bash
-c 5         # Space-separated
-c5          # Attached (no space)
```

**Order doesn't matter:**
```bash
-v -c 5 host
-c 5 -v host
host -v -c 5  # This won't work - options must come before positionals
```

**Stops at first non-option:**
```bash
prog -v host -c 5
# getopt() sees: -v
# optind points to: "host"
# "-c 5" are treated as positional arguments
```

**Use `--` to force end of options:**
```bash
prog -v -- -c 5
# getopt() sees: -v
# optind points to: "-c"
# Now "-c 5" are positional arguments
```

---

## Common Patterns

### Pattern 1: Parse Into Config Struct

**Best practice** - separate parsing from logic:

```c
typedef struct s_config {
    int verbose;
    int count;
    char *hostname;
} t_config;

int parse_args(int argc, char **argv, t_config *cfg) {
    int opt;

    // Set defaults
    cfg->count = 4;

    while ((opt = getopt(argc, argv, "vc:")) != -1) {
        switch (opt) {
            case 'v': cfg->verbose = 1; break;
            case 'c': cfg->count = atoi(optarg); break;  // TODO: validate
            case '?': return -1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: missing hostname\n");
        return -1;
    }
    cfg->hostname = argv[optind];
    return 0;
}

int main(int argc, char **argv) {
    t_config config = {0};

    if (parse_args(argc, argv, &config) < 0) {
        print_usage(argv[0]);
        return 1;
    }

    // Use config
    ping(config.hostname, config.count, config.verbose);
    return 0;
}
```

**Why this is good:**
- Separation of concerns (parsing vs logic)
- Easier to test
- Config struct can be passed around
- Defaults are explicit

### Pattern 2: Validation Helper

**Never trust optarg** - always validate:

```c
int parse_positive_int(const char *str, const char *opt_name) {
    char *endptr;
    long val;

    errno = 0;
    val = strtol(str, &endptr, 10);

    if (errno != 0) {
        fprintf(stderr, "Error: %s value too large\n", opt_name);
        return -1;
    }
    if (endptr == str) {
        fprintf(stderr, "Error: %s requires a number\n", opt_name);
        return -1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: %s has trailing characters\n", opt_name);
        return -1;
    }
    if (val <= 0 || val > INT_MAX) {
        fprintf(stderr, "Error: %s out of range\n", opt_name);
        return -1;
    }

    return (int)val;
}

// Usage:
case 'c':
    config->count = parse_positive_int(optarg, "count");
    if (config->count < 0)
        return -1;
    break;
```

**Why strtol() instead of atoi():**
- atoi() returns 0 on error (can't distinguish "0" from error)
- atoi() has no error detection
- strtol() sets errno on overflow
- strtol() tells you where parsing stopped (endptr)

### Pattern 3: Subcommands (like git)

For tools with subcommands (git clone, git commit, etc.):

```c
int main(int argc, char **argv) {
    const char *subcmd;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    subcmd = argv[1];

    if (strcmp(subcmd, "clone") == 0) {
        return cmd_clone(argc - 1, argv + 1);  // Shift argv
    } else if (strcmp(subcmd, "commit") == 0) {
        return cmd_commit(argc - 1, argv + 1);
    } else {
        fprintf(stderr, "Unknown command: %s\n", subcmd);
        return 1;
    }
}

int cmd_clone(int argc, char **argv) {
    // Now parse options specific to 'clone'
    int opt;
    while ((opt = getopt(argc, argv, "b:")) != -1) {
        // ...
    }
}
```

---

## Error Handling Best Practices

### Do's and Don'ts

**DO: Validate numeric arguments**
```c
case 'c':
    if (!is_valid_number(optarg)) {
        fprintf(stderr, "Error: -c requires a positive integer\n");
        return -1;
    }
    count = atoi(optarg);
    if (count <= 0 || count > MAX_COUNT) {
        fprintf(stderr, "Error: count must be 1-%d\n", MAX_COUNT);
        return -1;
    }
    break;
```

**DO: Check for required positional arguments**
```c
if (optind >= argc) {
    fprintf(stderr, "%s: missing required argument\n", argv[0]);
    print_usage(argv[0]);
    return 1;
}
```

**DO: Provide helpful error messages**
```c
case '?':
    if (optopt == 'c') {
        fprintf(stderr, "%s: option -%c requires an argument\n",
                argv[0], optopt);
    } else {
        fprintf(stderr, "%s: invalid option -%c\n",
                argv[0], optopt);
    }
    print_usage(argv[0]);
    return 1;
```

**DON'T: Use atoi() without validation**
```c
// BAD
case 'c':
    count = atoi(optarg);  // Returns 0 on error!
    break;

// GOOD
case 'c':
    if (!is_valid_number(optarg)) {
        fprintf(stderr, "Error: invalid count: %s\n", optarg);
        return -1;
    }
    count = atoi(optarg);
    break;
```

**DON'T: Forget to handle missing arguments**
```c
if (optind >= argc) {
    // Handle missing positional argument
}
```

### Standard Error Message Format

Follow GNU conventions:

```
progname: error message
progname: -c: invalid option
progname: option requires an argument -- 'c'
```

Example:
```c
fprintf(stderr, "%s: %s\n", argv[0], error_message);
fprintf(stderr, "%s: -%c: %s\n", argv[0], opt, error_message);
```

---

## getopt() vs getopt_long()

### Comparison

| Feature | getopt() | getopt_long() |
|---------|----------|---------------|
| **Standard** | POSIX | GNU extension |
| **Availability** | Universal | Linux, BSD, macOS |
| **Short options** | Yes (-v) | Yes (-v) |
| **Long options** | No | Yes (--verbose) |
| **Option clustering** | Yes (-vc) | Yes (-vc) |
| **Argument styles** | -c5, -c 5 | -c5, -c 5, --count=5, --count 5 |
| **Use case** | Simple tools | User-facing tools |

### When to Use Each

**Use getopt():**
- Maximum portability (POSIX-only systems)
- Simple tools with few options
- Internal/system utilities
- 42 school projects (if getopt_long not allowed)
- You want minimal dependencies

**Use getopt_long():**
- User-facing applications
- Tools with many options
- Professional/production software
- When you want self-documenting options (--verbose is clearer than -v)
- Modern Linux/Unix tools (it's the de facto standard)

### Migration from getopt() to getopt_long()

Easy - just add the long options array:

```c
// Before (getopt only)
while ((opt = getopt(argc, argv, "vc:")) != -1) {
    switch (opt) {
        case 'v': verbose = 1; break;
        case 'c': count = atoi(optarg); break;
    }
}

// After (getopt_long)
static struct option long_options[] = {
    {"verbose", no_argument,       NULL, 'v'},
    {"count",   required_argument, NULL, 'c'},
    {0, 0, 0, 0}
};

while ((opt = getopt_long(argc, argv, "vc:", long_options, NULL)) != -1) {
    // Switch statement stays THE SAME!
    switch (opt) {
        case 'v': verbose = 1; break;
        case 'c': count = atoi(optarg); break;
    }
}
```

**Key insight:** The switch statement doesn't change! Long options map to their short equivalents.

---

## Real-World Examples

### Example 1: ping (inetutils)

```c
// Simplified from inetutils ping.c
while ((opt = getopt(argc, argv, "c:i:s:t:vnRh")) != -1) {
    switch (opt) {
        case 'c':  // count
            count = parse_int(optarg);
            break;
        case 'i':  // interval
            interval = parse_float(optarg);
            break;
        case 's':  // packet size
            datalen = parse_int(optarg);
            break;
        case 't':  // TTL
            ttl = parse_int(optarg);
            break;
        case 'v':  // verbose
            verbose = 1;
            break;
        case 'n':  // numeric
            numeric = 1;
            break;
        case 'R':  // record route
            record_route = 1;
            break;
        case 'h':  // help
            usage();
            exit(0);
        default:
            usage();
            exit(1);
    }
}
```

### Example 2: ls (GNU coreutils)

```c
// Simplified from coreutils ls.c
static struct option const long_options[] = {
    {"all", no_argument, NULL, 'a'},
    {"human-readable", no_argument, NULL, 'h'},
    {"inode", no_argument, NULL, 'i'},
    {"size", no_argument, NULL, 's'},
    {"sort", required_argument, NULL, SORT_OPTION},
    {GETOPT_HELP_OPTION_DECL},
    {NULL, 0, NULL, 0}
};

while ((opt = getopt_long(argc, argv, "alhis1",
                          long_options, NULL)) != -1) {
    switch (opt) {
        case 'a': all_files = true; break;
        case 'l': format = long_format; break;
        case 'h': human_readable = true; break;
        // ...
    }
}
```

### Example 3: curl (simplified pattern)

```c
// Pattern from curl's argument parsing
const struct option longopts[] = {
    {"output", required_argument, NULL, 'o'},
    {"verbose", no_argument, NULL, 'v'},
    {"user", required_argument, NULL, 'u'},
    {"header", required_argument, NULL, 'H'},
    {NULL, 0, NULL, 0}
};

while ((opt = getopt_long(argc, argv, "vo:u:H:",
                          longopts, NULL)) != -1) {
    switch (opt) {
        case 'v': config->verbose++; break;  // Can repeat: -vvv
        case 'o': config->output = optarg; break;
        case 'u': parse_user_password(optarg, config); break;
        case 'H': add_header(optarg, config); break;
    }
}
```

---

## Common Pitfalls

### Pitfall 1: Not Checking optind

```c
// BAD
hostname = argv[1];  // Wrong! Might be an option

// GOOD
if (optind >= argc) {
    fprintf(stderr, "Error: missing hostname\n");
    exit(1);
}
hostname = argv[optind];
```

### Pitfall 2: Using atoi() Without Validation

```c
// BAD
case 'c':
    count = atoi(optarg);  // atoi("abc") returns 0 - no error!
    break;

// GOOD
case 'c':
    if (!is_valid_number(optarg)) {
        fprintf(stderr, "Error: count must be a number\n");
        exit(1);
    }
    count = atoi(optarg);
    break;
```

### Pitfall 3: Forgetting ':' in optstring

```c
// BAD - forgot the ':'
getopt(argc, argv, "vc")  // -c has no argument

// GOOD
getopt(argc, argv, "vc:")  // -c requires an argument
```

### Pitfall 4: Not Setting opterr

```c
// Without setting opterr, getopt prints its own messages:
// "invalid option -- 'x'"

// Set opterr = 0 to disable and provide your own messages:
opterr = 0;
```

### Pitfall 5: Modifying argv

```c
// BAD - don't modify argv
argv[i] = something_else;

// GOOD - getopt handles argv internally, you just read from it
hostname = argv[optind];  // Just read, don't write
```

### Pitfall 6: Missing break in Switch

```c
switch (opt) {
    case 'v':
        verbose = 1;
        // Missing break - falls through!
    case 'n':
        numeric = 1;
        break;
}

// Now -v sets both verbose AND numeric!
```

### Pitfall 7: Not Handling '?' Case

```c
// BAD - no error handling
while ((opt = getopt(argc, argv, "vc:")) != -1) {
    switch (opt) {
        case 'v': verbose = 1; break;
        case 'c': count = atoi(optarg); break;
        // Missing '?' case - errors are silently ignored!
    }
}

// GOOD
switch (opt) {
    case 'v': verbose = 1; break;
    case 'c': count = atoi(optarg); break;
    case '?':
        print_usage(argv[0]);
        exit(1);
}
```

---

## Advanced Techniques

### Technique 1: Counting Repeated Options

```c
int verbosity = 0;

while ((opt = getopt(argc, argv, "v")) != -1) {
    switch (opt) {
        case 'v':
            verbosity++;  // -v = 1, -vv = 2, -vvv = 3
            break;
    }
}

// Now: -v (level 1), -vv (level 2), -vvv (level 3)
```

### Technique 2: Mutually Exclusive Options

```c
int mode = 0;

while ((opt = getopt(argc, argv, "abc")) != -1) {
    switch (opt) {
        case 'a':
        case 'b':
        case 'c':
            if (mode != 0) {
                fprintf(stderr, "Error: options -a, -b, -c are mutually exclusive\n");
                exit(1);
            }
            mode = opt;
            break;
    }
}
```

### Technique 3: Custom Option Validation

```c
typedef int (*validator_t)(const char *);

typedef struct {
    char opt;
    validator_t validate;
    const char *error_msg;
} option_validator_t;

int validate_port(const char *str) {
    int port = atoi(str);
    return port >= 1 && port <= 65535;
}

int validate_percentage(const char *str) {
    int pct = atoi(str);
    return pct >= 0 && pct <= 100;
}

option_validator_t validators[] = {
    {'p', validate_port, "port must be 1-65535"},
    {'l', validate_percentage, "percentage must be 0-100"},
    {0, NULL, NULL}
};

void validate_option(char opt, const char *value) {
    for (int i = 0; validators[i].opt != 0; i++) {
        if (validators[i].opt == opt) {
            if (!validators[i].validate(value)) {
                fprintf(stderr, "Error: %s\n", validators[i].error_msg);
                exit(1);
            }
        }
    }
}
```

### Technique 4: Dependency Checking

```c
// After parsing, check dependencies
if (mode == MODE_SERVER && !port_specified) {
    fprintf(stderr, "Error: server mode requires -p <port>\n");
    exit(1);
}

if (use_ssl && !cert_file) {
    fprintf(stderr, "Error: SSL requires --cert <file>\n");
    exit(1);
}
```

### Technique 5: Environment Variable Defaults

```c
// Set defaults from environment before parsing
const char *env_count = getenv("PING_COUNT");
if (env_count) {
    config.count = atoi(env_count);
}

// Then parse args (which override environment)
parse_arguments(argc, argv, &config);
```

---

## Summary: Best Practices Checklist

**Parsing:**
- [ ] Use getopt() (or getopt_long() for long options)
- [ ] Set opterr = 0 and handle errors yourself
- [ ] Parse into a config struct
- [ ] Set defaults before parsing

**Validation:**
- [ ] Validate all numeric arguments (use strtol(), not atoi())
- [ ] Check for missing required arguments (optind < argc)
- [ ] Check for out-of-range values
- [ ] Validate string arguments (filenames, interfaces, etc.)

**Error Handling:**
- [ ] Handle '?' case in switch
- [ ] Provide helpful error messages
- [ ] Include program name in errors (argv[0])
- [ ] Print usage on errors

**Code Quality:**
- [ ] Separate parsing from business logic
- [ ] Use helper functions for validation
- [ ] Document option behavior
- [ ] Test all option combinations

**User Experience:**
- [ ] Provide -h/--help option
- [ ] Show examples in usage
- [ ] Support standard conventions (-v for verbose, -c for count, etc.)
- [ ] Consider supporting both short and long options

---

## References

- **POSIX.1-2017**: getopt() specification
- **GNU Coding Standards**: Argument parsing conventions
- **GNU libc manual**: getopt/getopt_long documentation
- **Source code to study:**
  - GNU coreutils: https://git.savannah.gnu.org/cgit/coreutils.git
  - inetutils: https://git.savannah.gnu.org/cgit/inetutils.git
  - util-linux: https://github.com/util-linux/util-linux

---

## Quick Reference

```c
// Basic template
#include <unistd.h>

int opt;
opterr = 0;
while ((opt = getopt(argc, argv, "v n c: t: h")) != -1) {
    switch (opt) {
        case 'v': /* flag */ break;
        case 'c': /* optarg has value */ break;
        case '?': /* error */ break;
    }
}
// Now: argv[optind] is first positional argument
```

```c
// getopt_long template
#include <getopt.h>

static struct option long_options[] = {
    {"verbose", no_argument,       NULL, 'v'},
    {"count",   required_argument, NULL, 'c'},
    {0, 0, 0, 0}
};

int opt;
while ((opt = getopt_long(argc, argv, "vc:", long_options, NULL)) != -1) {
    // Same switch as getopt!
}
```

---

**You now have everything you need to handle argument parsing like a professional.**

Stop reinventing the wheel. Use getopt(). Your future self will thank you.
