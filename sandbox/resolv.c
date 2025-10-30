#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int ac, char **av)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *p;
	int status;
	char ipstr[INET_ADDRSTRLEN];

	if (ac != 2)
	{
		fprintf(stderr, "Usage: %s <hostname>\n", av[0]);
		return 1;
	}

	// Initialize hints struct to zero
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;        // IPv4
	hints.ai_socktype = SOCK_RAW;     // Raw socket for ICMP
	hints.ai_protocol = IPPROTO_ICMP; // ICMP protocol

	// Resolve hostname to IP address
	status = getaddrinfo(av[1], NULL, &hints, &res);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return 2;
	}

	printf("IP addresses for %s:\n\n", av[1]);

	// Loop through all results and print them
	for (p = res; p != NULL; p = p->ai_next)
	{
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

		// Convert binary IP to string
		inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
		printf("  %s\n", ipstr);
	}

	// IMPORTANT: Free the linked list
	freeaddrinfo(res);

	return 0;
}