#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(void)
{
	struct addrinfo hints, *res;
	char ipstr[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo("google.com", NULL, &hints, &res);

	printf("Before freeaddrinfo:\n");
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
	inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
	printf("  IP address: %s\n", ipstr);
	printf("  ai_family: %d\n", res->ai_family);
	printf("  ai_socktype: %d\n", res->ai_socktype);

	// Free the memory
	freeaddrinfo(res);

	printf("\nAfter freeaddrinfo (USE-AFTER-FREE):\n");
	// DANGER: res is now a dangling pointer
	// But the data might still be there... for now
	printf("  ai_family: %d", res->ai_family);
	printf(" (might look OK!)\n");
	printf("  ai_socktype: %d", res->ai_socktype);
	printf(" (might look OK!)\n");

	// Try to read the IP again
	ipv4 = (struct sockaddr_in *)res->ai_addr;
	inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
	printf("  IP address: %s", ipstr);
	printf(" (might STILL look OK!)\n");

	printf("\n⚠️  This is UNDEFINED BEHAVIOR - just because it\n");
	printf("    printed correctly doesn't mean it's safe!\n");
	printf("    The memory could be reallocated at any moment.\n");

	// Let's trigger some allocations to corrupt the freed memory
	printf("\n--- Allocating new memory to potentially corrupt freed data ---\n");

	void *junk1 = malloc(1000);
	void *junk2 = malloc(1000);
	void *junk3 = malloc(1000);
	memset(junk1, 0xAA, 1000);
	memset(junk2, 0xBB, 1000);
	memset(junk3, 0xCC, 1000);

	printf("\nAfter allocations (USE-AFTER-FREE on corrupted memory):\n");
	printf("  ai_family: %d", res->ai_family);
	printf(" (probably garbage now)\n");
	printf("  ai_socktype: %d", res->ai_socktype);
	printf(" (probably garbage now)\n");

	// This might crash or print garbage
	printf("  Attempting to read IP... ");
	fflush(stdout);
	// Commenting out because this might actually crash:
	// inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
	printf("(skipped - would likely crash)\n");

	free(junk1);
	free(junk2);
	free(junk3);

	return 0;
}
