/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:58:49 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/27 18:03:49 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	print_bytes(uint8_t *bytes, int len)
{
	int	i;

	i = 0;
	while (i < len)
		printf("%02x", (uint8_t) bytes[i++]);
	printf("\n");
}

void	print_icmp(uint8_t *bytes, int len)
{
	int	i;

	i = 0;
	while (i < 8)
	{
		if (i == 0)
			printf("Type:\t");
		if (i == 1)
			printf("\nCode:\t");
		if (i == 2)
			printf("\nChksum:\t");
		if (i == 4)
			printf("\nRest:\t");
		printf("%02x", (uint8_t) bytes[i++]);
	}
	printf("\nPayload:\t");
	print_bytes(bytes + 8, len - 8);
}

void	print_usage(char *prog_name)
{
	fprintf(stderr, "Usage:\n\tsudo %s [-v] destination\n", prog_name);
}

void	print_addr(struct sockaddr_in *addr)
{
	printf("%s\n", inet_ntoa(addr->sin_addr));
}

time_t	elapsed_time(struct timeval start, struct timeval end)
{
	time_t			ret;
	struct timeval	elapsed;
	
	elapsed.tv_sec = end.tv_sec - start.tv_sec;
	elapsed.tv_usec = end.tv_usec - start.tv_usec;
	ret = elapsed.tv_sec * 1000 + elapsed.tv_usec;
	return (ret);
}