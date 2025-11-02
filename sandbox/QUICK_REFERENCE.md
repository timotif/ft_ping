# getopt() Quick Reference Card

**For when you need a fast answer without reading the full guide.**

---

## Basic Template

```c
#include <unistd.h>

int main(int argc, char **argv) {
    int opt;
    opterr = 0;  // Disable automatic errors

    while ((opt = getopt(argc, argv, "vc:h")) != -1) {
        switch (opt) {
            case 'v': /* flag */ break;
            case 'c': /* optarg = argument */ break;
            case 'h': /* help */ exit(0);
            case '?': /* error */ return 1;
        }
    }

    // Positional args start at argv[optind]
    if (optind >= argc) {
        fprintf(stderr, "Missing argument\n");
        return 1;
    }

    return 0;
}
```

---

## Option String Syntax

```c
getopt(argc, argv, "abc:d::")
//                    ^  ^  ^^
//                    |  |  |+- optional argument (GNU)
//                    |  +---- required argument
//                    +------- flag (no argument)
```

| Pattern | Meaning | Example Usage |
|---------|---------|---------------|
| `"v"` | Flag | `-v` |
| `"c:"` | Requires arg | `-c 5` or `-c5` |
| `"d::"` | Optional arg (rare) | `-d` or `-d3` |

---

## Global Variables

```c
extern char *optarg;  // Points to option argument
extern int optind;    // Index of next argv element
extern int opterr;    // 1=print errors, 0=silent
extern int optopt;    // Option char when error occurs
```

**Usage:**
```c
case 'c':
    int count = atoi(optarg);  // optarg = "5" for -c5
    break;

// After loop:
char *hostname = argv[optind];  // First positional arg
```

---

## Return Values

| Value | Meaning |
|-------|---------|
| `'v'`, `'c'`, etc. | Option found |
| `-1` | All options processed |
| `'?'` | Unknown option or missing arg |

---

## Common Patterns

### Parse into struct
```c
typedef struct {
    int verbose;
    int count;
    char *host;
} config_t;

int parse_args(int argc, char **argv, config_t *cfg) {
    int opt;
    while ((opt = getopt(argc, argv, "vc:")) != -1) {
        switch (opt) {
            case 'v': cfg->verbose = 1; break;
            case 'c': cfg->count = atoi(optarg); break;
            case '?': return -1;
        }
    }
    if (optind >= argc)
        return -1;
    cfg->host = argv[optind];
    return 0;
}
```

### Validate numbers
```c
case 'c':
    if (!is_valid_number(optarg)) {
        fprintf(stderr, "Error: -c requires a number\n");
        return -1;
    }
    count = atoi(optarg);
    break;
```

### Handle errors
```c
case '?':
    if (optopt == 'c') {
        fprintf(stderr, "Option -%c requires an argument\n", optopt);
    } else {
        fprintf(stderr, "Unknown option: -%c\n", optopt);
    }
    return -1;
```

### Count repeated options
```c
case 'v':
    verbosity++;  // -v=1, -vv=2, -vvv=3
    break;
```

---

## What getopt() Handles

✅ **Automatic:**
- Option clustering: `-vc` → `-v -c`
- Argument styles: `-c5` and `-c 5` both work
- Unknown options: returns `'?'`
- Missing arguments: returns `'?'` with optopt set

❌ **You must do:**
- Validate optarg (could be invalid!)
- Check for missing positional args
- Range checking on numbers
- Provide error messages

---

## Common Mistakes

### 1. Not checking optind
```c
// BAD
hostname = argv[1];

// GOOD
if (optind >= argc) {
    fprintf(stderr, "Missing hostname\n");
    return 1;
}
hostname = argv[optind];
```

### 2. Using atoi() without validation
```c
// BAD
count = atoi(optarg);  // Returns 0 on error!

// GOOD
if (!is_valid_number(optarg)) {
    fprintf(stderr, "Invalid number\n");
    return 1;
}
count = atoi(optarg);
```

### 3. Forgetting ':' in optstring
```c
// BAD
getopt(argc, argv, "vc")  // -c has no argument

// GOOD
getopt(argc, argv, "vc:")  // -c requires argument
```

### 4. Missing break in switch
```c
case 'c':
    count = atoi(optarg);
    break;  // ← DON'T FORGET!
```

---

## getopt_long() Template

```c
#include <getopt.h>

static struct option long_options[] = {
    {"verbose", no_argument,       NULL, 'v'},
    {"count",   required_argument, NULL, 'c'},
    {"help",    no_argument,       NULL, 'h'},
    {0, 0, 0, 0}  // Sentinel
};

int main(int argc, char **argv) {
    int opt;

    while ((opt = getopt_long(argc, argv, "vc:h",
                              long_options, NULL)) != -1) {
        switch (opt) {
            case 'v': /* verbose */ break;
            case 'c': /* count */ break;
            case 'h': /* help */ break;
            case '?': /* error */ return 1;
        }
    }

    return 0;
}
```

**Now both work:**
- `-v` and `--verbose`
- `-c 5` and `--count=5`
- `-h` and `--help`

---

## Debug Tips

```c
// Print what getopt sees
printf("optarg=%s, optind=%d, optopt=%c\n",
       optarg, optind, optopt);

// Print all arguments
for (int i = 0; i < argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

// Print positional args
for (int i = optind; i < argc; i++)
    printf("Positional: %s\n", argv[i]);
```

---

## Testing Checklist

Test these cases:
- [ ] `-v host` (flag)
- [ ] `-c 5 host` (space-separated)
- [ ] `-c5 host` (attached)
- [ ] `-vc5 host` (clustering)
- [ ] `host -v` (option after positional - should NOT work)
- [ ] `-c` (missing argument - should error)
- [ ] `-c abc host` (invalid number - should error)
- [ ] `-x host` (unknown option - should error)
- [ ] `-h` (help)
- [ ] No arguments (should error)

---

## When to Use What

| Scenario | Use |
|----------|-----|
| Simple tool, few options | `getopt()` |
| User-facing tool | `getopt_long()` |
| Maximum portability | `getopt()` |
| Self-documenting options | `getopt_long()` |
| 42 school (if allowed) | `getopt()` |

---

## One-Minute Summary

1. Include `<unistd.h>`
2. Set `opterr = 0`
3. Loop with `getopt(argc, argv, "vc:h")`
4. Switch on return value
5. Use `optarg` for option arguments
6. Use `argv[optind]` for positional arguments
7. Always validate `optarg`
8. Always check `optind < argc`

**Done. You now know getopt().**

---

## Full Examples

See:
- `getopt_tutorial.c` - Complete working example with comments
- `getopt_long_tutorial.c` - Extended with long options
- `README_ARGUMENT_PARSING.md` - Full guide
- `COMPARISON_CURRENT_VS_GETOPT.md` - Your code vs getopt()
- `REFACTORING_GUIDE.md` - Step-by-step migration

---

## Resources

- Man page: `man 3 getopt`
- POSIX spec: https://pubs.opengroup.org/onlinepubs/9699919799/
- GNU libc: https://www.gnu.org/software/libc/manual/html_node/Getopt.html

---

**Print this. Tape it to your wall. Never write a manual arg parser again.**
