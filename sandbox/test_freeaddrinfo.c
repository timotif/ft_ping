#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(void)
{
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo("google.com", NULL, &hints, &res) != 0)
		return 1;

	printf("Before freeaddrinfo:\n");
	printf("  res pointer = %p\n", (void *)res);
	printf("  res is NULL? %s\n", res == NULL ? "YES" : "NO");

	freeaddrinfo(res);

	printf("\nAfter freeaddrinfo:\n");
	printf("  res pointer = %p\n", (void *)res);
	printf("  res is NULL? %s\n", res == NULL ? "YES" : "NO");

	// This demonstrates that res is NOT nullified
	// The pointer still has the old address (dangling pointer)
	// But the memory it points to has been freed

	return 0;
}
