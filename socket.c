/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 18:30:09 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/01 14:55:58 by tfregni          ###   ########.fr       */
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
	// struct timeval	timeout;
	int				enable;

	// timeout.tv_sec = SOCKET_TIMEOUT;
	// timeout.tv_usec = 0;
	// if (setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, 
	// 		&timeout, sizeof(timeout)) < 0)
	// 	error(0, errno, "setsockopt (SO_RCVTIMEO)");
	// TODO: SO_BROADCAST if I want to support broadcast ping
	enable = 1;
	if (setsockopt(raw_socket, SOL_SOCKET, SO_TIMESTAMP,
			&enable, sizeof(enable)) != 0)
		error(0, errno, "setsockopt (SO_TIMESTAMP)");
	if (setsockopt(raw_socket, SOL_SOCKET, SO_BROADCAST, (char *) &enable, sizeof(enable)) != 0)
		error(0, errno, "setsockopt (SO_BROADCAST)");
	return (0);
}