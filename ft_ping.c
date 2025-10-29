/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:54:32 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/29 22:32:04 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

t_ft_ping	*g_ft_ping = NULL;

void	setup_destination(t_ft_ping *app, uint32_t dest_ip)
{
	struct sockaddr_in	*dest;

	dest = &app->dest_addr;
	memset(dest, 0, sizeof(*app->dest));
	dest->sin_family = AF_INET;
	dest->sin_addr.s_addr = dest_ip;
}

uint16_t	buffer_get_sequence(uint8_t *buffer, int len)
{
	t_ip_header		*ip_header;
	t_icmp_header	*icmp_header;
	int				offset;

	if (!buffer || !len)
		return (0);
	ip_header = (t_ip_header *)buffer;
	offset = ip_header->ihl * 4;
	icmp_header = (t_icmp_header *)(buffer + offset);
	return (icmp_get_sequence(icmp_header));
}

void	interrupt(int signum)
{
	float		loss;
	t_ft_ping	*app;

	app = g_ft_ping;
	(void) signum;
	loss = 0.0;
	if (g_ft_ping->sent_packets > 0)
		loss = 100 - (g_ft_ping->rcv_packets * 100 / g_ft_ping->sent_packets);
	/* --- 1.1.1.1 ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss */
	printf("--- %s ping statistics ---\n", g_ft_ping->dest);
	printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n",
		g_ft_ping->sent_packets, g_ft_ping->rcv_packets, loss);
	/* round-trip min/avg/max/stddev = 31.634/31.634/31.634/0.000 ms */
	printf("round-trip min/avg/max/stddev = %lld.%lld/%lld.%lld/%lld.%lld/%lld.%lld ms\n",
		app->stats[MIN] / 1000, app->stats[MIN] % 1000,
		app->stats[AVG] / 1000, app->stats[AVG] % 1000,
		app->stats[MAX] / 1000, app->stats[MAX] % 1000,
		app->stats[STDDEV] / 1000, app->stats[STDDEV] % 1000);
	if (g_ft_ping->socket >= 0)
		close(g_ft_ping->socket);
	g_ft_ping = NULL;
	exit (0);
}

int	main(int ac, char **av)
{
	uint32_t			dest_ip;
	t_ft_ping			app;

	g_ft_ping = &app;
	memset(&app, 0, sizeof(app));
	parse_args(ac, av, &app);
	// TODO: resolve through DNS
	if (inet_pton(AF_INET, app.dest, &dest_ip) != 1)
	{
		fprintf(stderr, "ping: unknown host\n");
		return (1);
	}
	setup_destination(&app, dest_ip);
	app.socket = init_socket();
	app.pid = getpid();
	printf("PING %s (%s): %d data bytes",
		av[1],
		app.dest,
		PAYLOAD_SIZE
		);
	if (app.flags[VERBOSE])
		printf(", id 0x%04x = %d", app.pid, app.pid);
	printf("\n");
	signal(SIGINT, interrupt);
	return (ping_loop(app.socket, &app));
}
