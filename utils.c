/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:58:49 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/27 11:03:54 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	print_bytes(uint8_t *bytes, int len)
{
	for (int i = 0; i < len; i++)
		printf("%02x", (uint8_t) bytes[i]);
	printf("\n");
}

void	print_usage(char *prog_name)
{
	fprintf(stderr, "Usage:\n\tsudo %s [-v] destination\n", prog_name);
}

void	print_addr(struct sockaddr_in *addr)
{
	printf("%s\n", inet_ntoa(addr->sin_addr));
}