/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:55:07 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/29 21:08:50 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	update_stats(t_ft_ping *app, long long time)
{
	double	delta;
	double	delta2;

	if (app->rcv_packets == 1 || app->stats[MIN] > time)
		app->stats[MIN] = time;
	if (app->rcv_packets == 1 || app->stats[MAX] < time)
		app->stats[MAX] = time;	
	delta = time - app->stats[AVG];
	app->stats[AVG] += delta / app->rcv_packets;
	delta2 = time - app->stats[AVG];
	app->variance_m2 = delta * delta2;
	if (app->rcv_packets > 1)
		app->stats[STDDEV] = (long long)sqrt(app->variance_m2 / app->rcv_packets);
}

void	ping_success(t_ip_header *ip_header, t_icmp_header *icmp_header,
		t_ft_ping *app)
{
	long long		time;
	struct timeval	send_time;

	app->rcv_packets++;
	memcpy(&send_time, app->recvbuffer + (ip_header->ihl * 4) + sizeof(t_icmp_header),
		sizeof(send_time));
	time = elapsed_time(send_time, app->end);
	update_stats(app, time);
	/* 64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.022 ms */
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%lld.%03lld ms\n",
		PACKET_SIZE, ip_get_source_addr(ip_header),
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

/* Payload: timestamp in host order + filler */
void	prepare_payload(void *payload, int size)
{
	struct timeval	timestamp;

	memset(payload, 0, size);
	gettimeofday(&timestamp, NULL);
	memcpy(payload, &timestamp, sizeof(timestamp));
	memset(payload + sizeof(timestamp), 0x42, size - sizeof(timestamp));
}

int	ping_loop(int sock, t_ft_ping *app)
{
	char			payload[PAYLOAD_SIZE];
	int				bytes;
	struct timeval	recv_time;

	bytes = 0;
	app->pid = getpid();
	while (1)
	{
		// Prepare packet (timestamp embedded in payload)
		memset(&app->end, 0, sizeof(app->end));
		memset(&recv_time, 0, sizeof(recv_time));
		prepare_payload(payload, PAYLOAD_SIZE);
		prepare_echo_request_packet(payload, app->sendbuffer, ++app->sequence,
			app->pid);
		// print_icmp(app->sendbuffer, PACKET_SIZE); // DEBUG
		if (send_packet(sock, app->sendbuffer, &app->dest_addr) < 0)
			continue ;
		app->sent_packets++;
		bytes = receive_packet(sock, app->recvbuffer, sizeof(app->recvbuffer),
				&app->reply_addr, app->sequence, &recv_time);
		if (recv_time.tv_sec != 0)
			app->end = recv_time;
		else
			gettimeofday(&app->end, NULL);
		// print_ip(app->recvbuffer, bytes); // DEBUG
		if (bytes < 0)
		{
			if (errno == EWOULDBLOCK)
				printf("Request timeout for icmp_seq=%d\n", app->sequence);
			else
				perror("recvfrom");
		}
		else
			process_packet(bytes, app);
		sleep(1);
	}
	return (0);
}