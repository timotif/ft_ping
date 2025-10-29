/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 18:30:09 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/29 20:51:22 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"


int	init_socket(void)
{
	int				raw_socket;

	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_socket <= 0)
	{
		perror("socket");
		fprintf(stderr, "Hint: raw sockets require root privileges\n");
		exit (1);
	}
	if (set_socket_options(raw_socket))
	{
		perror("socket");
		exit (1);
	}
	return (raw_socket);
}

int	set_socket_options(int raw_socket)
{
	int				error;
	struct timeval	timeout;
	int				enable;

	error = 0;
	timeout.tv_sec = SOCKET_TIMEOUT;
	timeout.tv_usec = 0;
	if (setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, 
			&timeout, sizeof(timeout)) < 0)
		error = 1;
	enable = 1;
	if (setsockopt(raw_socket, SOL_SOCKET, SO_TIMESTAMP,
			&enable, sizeof(enable)) < 0)
		error = 1;
	return (error);
}