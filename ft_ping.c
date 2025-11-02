/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:54:32 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/02 21:56:10 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

t_ft_ping	*g_ft_ping = NULL;

void	setup_destination(t_ft_ping *app, uint32_t dest_ip)
{
	struct sockaddr_in	*dest;

	dest = &app->dest_addr;
	memset(dest, 0, sizeof(*app->hostname));
	dest->sin_family = AF_INET;
	dest->sin_addr.s_addr = dest_ip;
}

t_icmp_header	*icmp_get_header(uint8_t *buffer, size_t len)
{
	t_ip_header		*ip_header;
	int				offset;

	if (!ip_is_valid(buffer, len))
		return (NULL);
	ip_header = (t_ip_header *)buffer;
	offset = ip_header->ihl * 4;
	return ((t_icmp_header *)(buffer + offset));
}

int16_t	buffer_get_sequence(uint8_t *buffer, size_t len)
{
	t_icmp_header	*icmp_header;

	icmp_header = icmp_get_header(buffer, len);
	if (!icmp_header)
		return (-1);
	return ((int16_t)icmp_get_sequence(icmp_header));
}

void	clean_up()
{
	float		loss;
	t_ft_ping	*app;
	
	app = g_ft_ping;
	loss = 0.0;
	if (g_ft_ping->sent_packets > 0)
	loss = 100 - (g_ft_ping->rcv_packets * 100 / g_ft_ping->sent_packets);
	/*	--- 1.1.1.1 ping statistics ---
	1 packets transmitted, 1 packets received, 0% packet loss */
	printf("--- %s ping statistics ---\n", g_ft_ping->hostname);
	printf("%d packets transmitted, %d packets received, ", g_ft_ping->sent_packets, g_ft_ping->rcv_packets);
	if (app->dup_packets)
		printf("+%d duplicates, ", app->dup_packets);
	printf("%.1f%% packet loss\n", loss);
	/* round-trip min/avg/max/stddev = 31.634/31.634/31.634/0.000 ms */
	printf("round-trip min/avg/max/stddev = %lld.%lld/%lld.%lld/%lld.%lld/%lld.%lld ms\n",
	app->stats[MIN] / 1000, app->stats[MIN] % 1000,
	app->stats[AVG] / 1000, app->stats[AVG] % 1000,
	app->stats[MAX] / 1000, app->stats[MAX] % 1000,
	app->stats[STDDEV] / 1000, app->stats[STDDEV] % 1000);
	if (g_ft_ping->socket >= 0)
	close(g_ft_ping->socket);
	freeaddrinfo(g_ft_ping->res);
	g_ft_ping = NULL;
		exit (0);
}
		
void	interrupt(int signum)
{
	(void) signum; // Required by signal() but unused
	g_ft_ping->stop = 1;
}

void	print_addresses(struct addrinfo *res)
{
	struct addrinfo		*cur;
	struct sockaddr_in	*dest_addr;
	char				ip_str[INET_ADDRSTRLEN];
	
	for (cur = res; cur; cur = cur->ai_next)
	{
		dest_addr = (struct sockaddr_in *)cur->ai_addr;
		inet_ntop(cur->ai_family, &(dest_addr->sin_addr), ip_str,
			sizeof(ip_str));
		printf("    %s\n", ip_str);
	}
}

int	resolv_hostname(const char *hostname, t_ft_ping *app)
{
	int					status;
	struct addrinfo		hints;
	struct sockaddr_in	*dest_addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	status = getaddrinfo(hostname, NULL, &hints, &app->res);
	dest_addr = (struct sockaddr_in *)app->res->ai_addr;
	inet_ntop(app->res->ai_family, &(dest_addr->sin_addr), app->ip_str,
			sizeof(app->ip_str));
	if (!status)
		app->dest_addr = *(struct sockaddr_in *)app->res->ai_addr;
	return (status);
}

static void	init_app(t_ft_ping *app)
{
	memset(app, 0, sizeof(*app));
	app->packet_size = PACKET_SIZE;
	app->flags[INTERVAL] = INTERVAL_MS;
}

int	main(int ac, char **av)
{
	t_ft_ping			app;

	g_ft_ping = &app;
	init_app(&app);
	parse_args(ac, av, &app);
	if (resolv_hostname(app.hostname, &app) != 0)
	{
		freeaddrinfo(app.res);
		app.res = NULL;
		fprintf(stderr, "ping: unknown host\n");
		return (1);
	}
	app.socket = init_socket();
	app.pid = getpid();
	printf("PING %s (%s): %ld data bytes",
		app.hostname,
		app.ip_str,
		app.packet_size - ICMP_HEADER_SIZE
		);
	if (app.flags[VERBOSE])
		printf(", id 0x%04x = %d", app.pid, app.pid);
	printf("\n");
	signal(SIGINT, interrupt);
	return (ping_loop(&app));
}

/*ping: Lacking privilege for icmp socket.*/
/* TODO: duplicates (ex with broadcast) */