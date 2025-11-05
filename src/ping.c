/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:55:07 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/05 14:15:48 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/* Using Welford's algorithm to update as we go */
void	update_stats(t_ft_ping *app, long long time)
{
	double	delta;
	double	delta2;

	if (app->rcv_packets == 1 || app->stats[MIN] > time)
		app->stats[MIN] = time;
	if (app->rcv_packets == 1 || app->stats[MAX] < time)
		app->stats[MAX] = time;
	// Welford's algorithm
	delta = time - app->stats[AVG];
	app->stats[AVG] += delta / app->rcv_packets;
	delta2 = time - app->stats[AVG];
	app->variance_m2 += delta * delta2;
	if (app->rcv_packets > 1)
	app->stats[STDDEV] = (long long)sqrt(app->variance_m2 / app->rcv_packets);
}

void	ping_success(t_ip_header *ip_header, t_ft_ping *app, int rcv_seq)
{
	long long		time;
	struct timeval	send_time;
	int				dup;

	dup = 0;
	if (bitmap_test(app->rcv_map, rcv_seq))
	{
		app->dup_packets++;
		dup = 1;
	}
	else
	{
		bitmap_set(app->rcv_map, rcv_seq);
		app->rcv_packets++;
	}
	// The packet's validity is checked in process_packet
	memcpy(&send_time, app->recvbuffer + (ip_header->ihl * 4)
		+ sizeof(t_icmp_header), sizeof(send_time));
	time = elapsed_time(send_time, app->end);
	update_stats(app, time);
	if (app->options[FLOOD] && !app->options[QUIET])
		putchar('\b');
	else
		print_echo(app->packet_size, ip_header, rcv_seq, time, dup);
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

/**
 * Send ICMP Echo Request packet
 * Exits on send failure
 */
static void	send_echo(t_ft_ping *app)
{
	char			payload[app->packet_size - ICMP_HEADER_SIZE];

	memset(&app->end, 0, sizeof(app->end));
	// Prepare packet (timestamp embedded in payload)
	prepare_payload(payload, app->packet_size - ICMP_HEADER_SIZE);
	prepare_echo_request_packet(payload, app->sendbuffer, app->sequence,
		app->pid);
	if (send_packet(app->socket, app->sendbuffer, &app->dest_addr) < 0)
		exit (1);
	if (app->options[FLOOD] && !app->options[QUIET])
		putchar('.');
	app->sent_packets++;
}

/* Receive packet, validate sequence number and process */
void	handle_packet_reception(t_ft_ping *app)
{
	int	bytes;
	int	rcv_seq;

	bytes = receive_packet(app->socket, app->recvbuffer,
			sizeof(app->recvbuffer), &app->reply_addr, &app->end);
	if (bytes < 0)
	{
		if (errno == EWOULDBLOCK)
			printf("Request timeout for icmp_seq=%d\n", app->sequence);
		else
			perror("recvfrom");
	}
	else
	{
		rcv_seq = buffer_get_sequence(app->recvbuffer, bytes);
		if (rcv_seq <= app->sequence && rcv_seq >= 0)
			process_packet(bytes, app, rcv_seq);
	}
	if (app->options[COUNT] && app->rcv_packets >= app->options[COUNT])
		app->stop = 1;
	if (app->options[TIMEOUT] && ping_timeout(&app->start, app->options[TIMEOUT]))
		app->stop = 1;
}

void	handle_select_error(void)
{
	if (errno != EINTR)
	{
		fprintf(stderr, "ft_ping: select failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void	send_next_packet(t_ft_ping *app, struct timeval *last)
{
	if (app->options[COUNT] && app->sent_packets >= app->options[COUNT])
		return ;
	if (app->options[TIMEOUT] && ping_timeout(&app->start, app->options[TIMEOUT]))
	{
		app->stop = 1;
		return ;
	}
	app->sequence++;
	send_echo(app);
	gettimeofday(last, NULL);
}

void	ping_preload(t_ft_ping *app, struct timeval *last)
{
	while (app->sent_packets < app->options[PRELOAD])
	{
		app->sequence++;
		send_echo(app);
	}
	gettimeofday(last, NULL);
}

int	ping_loop(t_ft_ping *app)
{
	struct timeval	interval, last, resp_time;
	fd_set			fdset;
	t_wait_result	wait_result;

	signal(SIGINT, interrupt);
	initialize_timing(app->options[INTERVAL], &interval, &last, &resp_time);
	gettimeofday(&app->start, NULL);
	app->sequence = 0;
	send_echo(app);
	if (app->options[PRELOAD])
		ping_preload(app, &last);
	while (1 && !app->stop)
	{
		FD_ZERO(&fdset);
		FD_SET(app->socket, &fdset);
		calculate_timeout_remaining(&resp_time, &last, &interval);
		wait_result = select(app->socket + 1, &fdset, NULL, NULL, &resp_time);
		if (wait_result == WAIT_ERROR)
			handle_select_error();
		else if (wait_result == WAIT_READY)
			handle_packet_reception(app);
		else // WAIT_TIMEOUT - time to send the next packet
			send_next_packet(app, &last);
	}
	clean_up();
	return (0);
}
