#!/bin/bash
usage="Usage:
	sudo ./ft_ping [-v] destination"

test() {
	echo "=== TEST: $1 ==="
	run="$1"
	output=$($run 2>&1)
	echo "*** OUTPUT ***"
	echo "$output"
	echo "**************"
	echo
}

make re
test ./ft_ping
test "./ft_ping -?"
test "./ft_ping 1.1.1.1"
test "sudo ./ft_ping"
test "sudo ./ft_ping 1.1.1.1"
test "sudo ./ft_ping 1.bla"
test "sudo ./ft_ping -g"
test "sudo ./ft_ping - g"
test "sudo ./ft_ping -v"