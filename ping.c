/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:55:07 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/28 18:00:15 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"


void	ping_success(t_ip_header *ip_header, t_icmp_header *icmp_header, 
		int bytes, t_ft_ping *app)
{
	long long	time;

	app->rcv_packets++;
	time = elapsed_time(app->start, app->end);
	/* 64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.022 ms */
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%lld.%03lld ms\n",
		bytes, ip_get_source_addr(ip_header), 
		icmp_get_sequence(icmp_header), 
		ip_header->ttl, time / 1000, time % 1000);	
}

void	ping_fail(t_ip_header *ip_header, t_icmp_header *icmp_header, 
		int bytes, t_ft_ping *app)
{
	(void) bytes;
	switch (icmp_header->type)
	{
		case ICMP_DEST_UNREACH:
			if (ntohs(icmp_header->un.echo.id) == app->pid)
				printf("From %s icmp_seq=%d Destination Unreachable\n",
					ip_get_source_addr(ip_header), 
					icmp_get_sequence(icmp_header));
			return;
		default:
			break;
	}
}

int	ping_loop(int sock, t_ft_ping *app)
{
	char	payload[PAYLOAD_SIZE];
	int		bytes;

	bytes = 0;
	app->pid = getpid();
	printf("PID: %d\n", app->pid);
	while (1)
	{
		// Prepare packet
		memset(payload, 0x42, PAYLOAD_SIZE);
		memset(&app->start, 0, sizeof(app->start));
		memset(&app->end, 0, sizeof(app->end));
		prepare_echo_request_packet(payload, PAYLOAD_SIZE, app->sendbuffer, 
				++app->sequence, app->pid);
		// print_bytes(app->sendbuffer, PACKET_SIZE, "SEND BUFFER");
		gettimeofday(&app->start, NULL);
		if (send_packet(sock, app->sendbuffer, &app->dest_addr) < 0)
			continue;
		app->sent_packets++;
		bytes = receive_packet(sock, app->recvbuffer, sizeof(app->recvbuffer),
				&app->reply_addr, app->sequence);
		gettimeofday(&app->end, NULL);
		if (bytes < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				printf("Request timeout for icmp_seq=%d\n", 
					app->sent_packets--);
			else
				perror("recvfrom");
		}
		else
			process_packet(bytes, app);
		sleep(1);
	}
	return (0);
}