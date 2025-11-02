# Makefile for argument parsing tutorials

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic
LDFLAGS =

# Tutorial programs
PROGRAMS = getopt_tutorial getopt_long_tutorial

all: $(PROGRAMS)
	@echo ""
	@echo "=========================================="
	@echo "Argument Parsing Tutorials Compiled!"
	@echo "=========================================="
	@echo ""
	@echo "Run the examples:"
	@echo "  ./getopt_tutorial -h"
	@echo "  ./getopt_long_tutorial -h"
	@echo ""
	@echo "Read the guide:"
	@echo "  less README_ARGUMENT_PARSING.md"
	@echo ""
	@echo "Try some commands:"
	@echo "  ./getopt_tutorial -v -c 5 localhost"
	@echo "  ./getopt_tutorial -vc5 -t128 8.8.8.8"
	@echo "  ./getopt_long_tutorial --verbose --count=10 google.com"
	@echo "  ./getopt_long_tutorial -v --count=5 --ttl=64 localhost"
	@echo ""

getopt_tutorial: getopt_tutorial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

getopt_long_tutorial: getopt_long_tutorial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(PROGRAMS)

test: all
	@echo "=========================================="
	@echo "Running Basic Tests"
	@echo "=========================================="
	@echo ""
	@echo "Test 1: Basic usage (getopt)"
	./getopt_tutorial -v localhost
	@echo ""
	@echo "Test 2: Option with argument (getopt)"
	./getopt_tutorial -c 10 8.8.8.8
	@echo ""
	@echo "Test 3: Option clustering (getopt)"
	./getopt_tutorial -vc5 -t128 google.com
	@echo ""
	@echo "Test 4: Long options with = (getopt_long)"
	./getopt_long_tutorial --verbose --count=5 localhost
	@echo ""
	@echo "Test 5: Long options with space (getopt_long)"
	./getopt_long_tutorial --count 10 --ttl 128 8.8.8.8
	@echo ""
	@echo "Test 6: Mixed short and long (getopt_long)"
	./getopt_long_tutorial -v --count=5 -t 64 google.com
	@echo ""
	@echo "=========================================="
	@echo "Error Handling Tests"
	@echo "=========================================="
	@echo ""
	@echo "Test 7: Missing hostname (should fail)"
	-./getopt_tutorial -v
	@echo ""
	@echo "Test 8: Unknown option (should fail)"
	-./getopt_tutorial -x localhost
	@echo ""
	@echo "Test 9: Missing argument (should fail)"
	-./getopt_tutorial -c
	@echo ""
	@echo "Test 10: Invalid number (should fail)"
	-./getopt_tutorial -c abc localhost
	@echo ""
	@echo "=========================================="
	@echo "All tests completed"
	@echo "=========================================="

demo: all
	@echo "=========================================="
	@echo "Interactive Demo"
	@echo "=========================================="
	@echo ""
	@echo "This will run various examples to show how the parsing works."
	@echo "Press Enter to continue between examples..."
	@echo ""
	@echo "[Example 1] Simple flag: -v"
	@read dummy
	./getopt_tutorial -v localhost
	@echo ""
	@echo "[Example 2] Multiple flags: -v -n"
	@read dummy
	./getopt_tutorial -v -n localhost
	@echo ""
	@echo "[Example 3] Option with space: -c 5"
	@read dummy
	./getopt_tutorial -c 5 localhost
	@echo ""
	@echo "[Example 4] Option attached: -c5"
	@read dummy
	./getopt_tutorial -c5 localhost
	@echo ""
	@echo "[Example 5] Clustering: -vc5"
	@read dummy
	./getopt_tutorial -vc5 localhost
	@echo ""
	@echo "[Example 6] Multiple options: -vc5 -t128"
	@read dummy
	./getopt_tutorial -vc5 -t128 localhost
	@echo ""
	@echo "[Example 7] Long options with =: --count=5"
	@read dummy
	./getopt_long_tutorial --count=5 localhost
	@echo ""
	@echo "[Example 8] Long options with space: --count 5"
	@read dummy
	./getopt_long_tutorial --count 5 localhost
	@echo ""
	@echo "[Example 9] Mixed: -v --count=5 --ttl=64"
	@read dummy
	./getopt_long_tutorial -v --count=5 --ttl=64 localhost
	@echo ""
	@echo "[Example 10] Everything combined: -vc5 --ttl=128 --interface eth0"
	@read dummy
	./getopt_long_tutorial -vc5 --ttl=128 --interface eth0 localhost
	@echo ""
	@echo "=========================================="
	@echo "Demo complete!"
	@echo "=========================================="

help:
	@echo "Argument Parsing Tutorial Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make           Build all examples"
	@echo "  make clean     Remove compiled programs"
	@echo "  make test      Run automated tests"
	@echo "  make demo      Run interactive demonstration"
	@echo "  make help      Show this help message"
	@echo ""
	@echo "Programs:"
	@echo "  getopt_tutorial       - Basic POSIX getopt() example"
	@echo "  getopt_long_tutorial  - GNU getopt_long() example"
	@echo ""
	@echo "Documentation:"
	@echo "  README_ARGUMENT_PARSING.md - Complete reference guide"

.PHONY: all clean test demo help
