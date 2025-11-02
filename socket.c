/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 18:30:09 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/02 21:26:35 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"


int	init_socket(void)
{
	int				raw_socket;

	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_socket <= 0)
	{
		perror("ping");
		fprintf(stderr, "Lacking privilege for icmp socket.\n");
		exit (1);
	}
	set_socket_options(raw_socket);
	return (raw_socket);
}

int	set_socket_options(int raw_socket)
{
	int	enable;

	enable = 1;
	if (setsockopt(raw_socket, SOL_SOCKET, SO_TIMESTAMP,
			&enable, sizeof(enable)) != 0)
	{
		fprintf(stderr, "ft_ping: setsockopt (SO_TIMESTAMP): %s\n",
			strerror(errno));
	}
	if (setsockopt(raw_socket, SOL_SOCKET, SO_BROADCAST,
			(char *)&enable, sizeof(enable)) != 0)
	{
		fprintf(stderr, "ft_ping: setsockopt (SO_BROADCAST): %s\n",
			strerror(errno));
	}
	if (g_ft_ping->flags[TTL])
	{
		if (setsockopt(raw_socket, IPPROTO_IP, IP_TTL,
				&g_ft_ping->flags[TTL], sizeof(g_ft_ping->flags[TTL]) != 0))
			fprintf(stderr, "ft_ping: setsockopt (IP_TTL): %s\n",
				strerror(errno));
	}
	return (0);
}