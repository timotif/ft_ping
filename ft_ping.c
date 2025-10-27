/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:54:32 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/27 21:00:36 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

t_ft_ping	*g_ft_ping = NULL;

int	init_socket(void)
{
	int				raw_socket;
	struct timeval	timeout;

	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_socket <= 0)
	{
		perror("socket");
		fprintf(stderr, "Hint: raw sockets require root privileges\n");
		exit (1);
	}
	timeout.tv_sec = SOCKET_TIMEOUT;
	timeout.tv_usec = 0;
	if (setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, 
			&timeout, sizeof(timeout)) < 0)
	{
		perror("socket");
		exit (1);
	}
	return (raw_socket);
}

void	setup_destination(t_ft_ping *app, uint32_t dest_ip)
{
	struct sockaddr_in	*dest;
	
	dest = &app->dest_addr;
	memset(dest, 0, sizeof(*app->dest));
	dest->sin_family = AF_INET;
	dest->sin_addr.s_addr = dest_ip;
}

uint32_t	calculate_checksum(uint16_t *data, uint32_t len)
{
	register long	sum;

	sum = 0;
	while (len > 1)
	{
		sum += *data++;
		len -= 2;
	}
	if (len > 0)
		sum += *(uint8_t*)data;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

void	prepare_echo_request_packet(void *payload, size_t payload_len, 
		uint8_t *sendbuffer, int seq, pid_t pid)
{
	t_icmp_header	*packet;
	size_t			packet_size;
	
	packet_size = PACKET_SIZE;
	memset(sendbuffer, 0, packet_size);
	packet = (t_icmp_header*) sendbuffer;
	packet->type = ICMP_ECHO; // 8
	packet->code = 0;
	packet->un.echo.id = pid;
	packet->un.echo.sequence = seq;
	memcpy(sendbuffer + sizeof(t_icmp_header), payload, payload_len);
	assert(packet->checksum == 0);
	packet->checksum = 0;
	packet->checksum = calculate_checksum((uint16_t*) packet, packet_size);
}

int	send_packet(int sock, uint8_t *sendbuffer, struct sockaddr_in *addr)
{
	int bytes;
	
	bytes = sendto(sock, sendbuffer, PACKET_SIZE, 
		0, (struct sockaddr *)addr, sizeof(*addr));
	if (bytes < 0)
		perror("send packet");
	return (bytes);
}

int	receive_packet(int sock, uint8_t *recvbuffer, size_t bufsize, 
		struct sockaddr_in *reply_addr)
{
	int	bytes;

	socklen_t reply_addr_len = sizeof(*reply_addr);
	memset(reply_addr, 0, reply_addr_len);
	bytes = recvfrom(sock, recvbuffer, bufsize, 0, 
				(struct sockaddr *) reply_addr, &reply_addr_len);
	return (bytes);
}

void	process_packet(int bytes, t_ft_ping *app)
{
	int				offset;
	t_ip_header		*ip_header;
	t_icmp_header	*icmp_header;
	
	ip_header = (t_ip_header*) app->recvbuffer;
	offset = ip_header->ihl * 4;
	icmp_header = (struct icmphdr *)(app->recvbuffer + offset);
	// print_icmp((uint8_t *)icmp_header, bytes - offset);
	if (icmp_header->type == ICMP_ECHOREPLY && 
		icmp_header->un.echo.id == app->pid) // TODO: save pid in app
	{
		app->rcv_packets++;
		time_t time = elapsed_time(app->start, app->end);
		/* 64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.022 ms */
		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%ld.%03lu ms\n",
			bytes, inet_ntoa(app->reply_addr.sin_addr), icmp_header->un.echo.sequence, 
			ip_header->ttl, time / 1000, time % 1000);
	}
}

int	ping_loop(int sock, t_ft_ping *app)
{
	char	payload[PAYLOAD_SIZE];
	int		bytes;

	app->pid = getpid() & 0xffff;
	while (1)
	{
		// Prepare packet
		memset(payload, 0x42, PAYLOAD_SIZE);
		memset(&app->start, 0, sizeof(app->start));
		memset(&app->end, 0, sizeof(app->end));
		prepare_echo_request_packet(payload, PAYLOAD_SIZE, app->sendbuffer, 
				app->sent_packets++, app->pid);
		// print_bytes(app->sendbuffer, PACKET_SIZE);
		gettimeofday(&app->start, NULL);
		if (send_packet(sock, app->sendbuffer, &app->dest_addr) < 0)
			continue;
		bytes = receive_packet(sock, app->recvbuffer, sizeof(app->recvbuffer),
				&app->reply_addr);
		gettimeofday(&app->end, NULL);
		if (bytes < 0)
			perror("recvfrom");
		else
			process_packet(bytes, app);
		sleep(1);
	}
	return (0);
}

void	interrupt(int signum)
{
	(void) signum;
	float	loss;
	
	loss = 0.0;
	if (g_ft_ping->sent_packets > 0)
		loss = 100 - (g_ft_ping->rcv_packets * 100 / g_ft_ping->sent_packets);
	printf("--- %s ping statistics ---\n", g_ft_ping->dest);
	printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n",
		g_ft_ping->sent_packets, g_ft_ping->rcv_packets, loss);
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
		fprintf(stderr, "Invalid address: %s\n", app.dest);
		return (1);
	}
	setup_destination(&app, dest_ip);
	app.socket = init_socket();
	printf("PING %s (%s): %d data bytes\n",
		av[1],
		app.dest,
		PACKET_SIZE
		);
	signal(SIGINT, interrupt);
	return (ping_loop(app.socket, &app));
}
