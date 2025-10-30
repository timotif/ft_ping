#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

void example_leak(void)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	getaddrinfo("google.com", NULL, &hints, &res);
	// Function returns without calling freeaddrinfo(res)
	// → MEMORY LEAK: allocated memory is orphaned
}

void example_dangling(void)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	getaddrinfo("google.com", NULL, &hints, &res);
	freeaddrinfo(res);  // Memory freed ✓
	// res still contains old address
	// → DANGLING POINTER: pointer references freed memory

	// If we tried to use res here = undefined behavior
	// But NO LEAK - memory was freed!
}

void example_correct(void)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	getaddrinfo("google.com", NULL, &hints, &res);
	freeaddrinfo(res);
	res = NULL;  // Good practice: nullify dangling pointer
	// Now if we accidentally use res, we get NULL dereference
	// (easier to debug than use-after-free)
}

int main(void)
{
	printf("Testing memory behaviors:\n\n");

	printf("1. Leak example (memory not freed):\n");
	example_leak();
	printf("   → Memory still allocated, no way to free it\n\n");

	printf("2. Dangling pointer (memory freed, pointer not nulled):\n");
	example_dangling();
	printf("   → Memory freed, but pointer has stale address\n\n");

	printf("3. Correct usage (memory freed, pointer nulled):\n");
	example_correct();
	printf("   → Memory freed AND pointer set to NULL\n\n");

	return 0;
}
