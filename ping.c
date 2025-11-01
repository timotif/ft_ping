/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:55:07 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/01 18:09:47 by tfregni          ###   ########.fr       */
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

void	ping_success(t_ip_header *ip_header, t_icmp_header *icmp_header,
		t_ft_ping *app, int rcv_seq)
{
	long long		time;
	struct timeval	send_time;
	int				dup;

	(void) icmp_header; // TODO: maybe erase
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
	memcpy(&send_time, app->recvbuffer + (ip_header->ihl * 4) + sizeof(t_icmp_header),
		sizeof(send_time));
	time = elapsed_time(send_time, app->end);
	update_stats(app, time);
	/* 64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.022 ms */
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%lld.%03lld ms",
		PACKET_SIZE, ip_get_source_addr(ip_header),
		rcv_seq,
		ip_header->ttl, time / 1000, time % 1000);
	if (dup)
		printf(" (DUP!)");
	printf("\n");
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

int	send_echo(t_ft_ping *app)
{
	char			payload[PAYLOAD_SIZE];
	
	memset(&app->end, 0, sizeof(app->end));
	// Prepare packet (timestamp embedded in payload)
	prepare_payload(payload, PAYLOAD_SIZE);
	prepare_echo_request_packet(payload, app->sendbuffer, app->sequence,
		app->pid);
	// print_icmp(app->sendbuffer, PACKET_SIZE); // DEBUG
	if (send_packet(app->socket, app->sendbuffer, &app->dest_addr) < 0)
		return (-1);
	app->sent_packets++;
	return (0);
}

/* Normalizes a timeval-like structure's microseconds field so the pair 
(tv_sec, tv_usec) represents the same total time but with tv_usec in the 
canonical range [0, 10^6)*/
static void normalize_timeval(struct timeval *t)
{
  long long usec = (long long)t->tv_usec;
  long long sec = (long long)t->tv_sec;

  long long carry = usec / 1000000LL;
  usec -= carry * 1000000LL;
  sec += carry;

  /* if usec negative after division adjust one more second */
  if (usec < 0) {
    usec += 1000000LL;
    sec -= 1;
  }

  t->tv_sec = (time_t)sec;
  t->tv_usec = (suseconds_t)usec;
}

int	ping_loop(t_ft_ping *app)
{
	int				bytes;
	int				rcv_seq;
	struct timeval	interval, now, last, resp_time;
	fd_set			fdset;
	int				fd;

	memset(&interval, 0, sizeof(interval));
	memset(&now, 0, sizeof(now));
	memset(&resp_time, 0, sizeof(resp_time));
	memset(&last, 0, sizeof(last));
	interval.tv_sec = INTERVAL / 1000;
	interval.tv_usec = (INTERVAL % 1000) * 1000;
	gettimeofday(&last, NULL);
	app->sequence = 0;
	send_echo(app);
	while (1 && !app->stop)
	{
		FD_ZERO(&fdset);
		FD_SET(app->socket, &fdset);
		gettimeofday(&now, NULL);
		resp_time.tv_sec = last.tv_sec + interval.tv_sec - now.tv_sec;
		resp_time.tv_usec = last.tv_usec + interval.tv_usec - now.tv_usec;
		normalize_timeval(&resp_time);
		if (resp_time.tv_sec < 0)
			resp_time.tv_sec = resp_time.tv_usec = 0;
		// printf("resp_time: %ld s, %ld us\n", resp_time.tv_sec, resp_time.tv_usec); // DEBUG
		fd = select(app->socket + 1, &fdset, NULL, NULL, &resp_time);
		if (fd < 0)
		{
			if (errno != EINTR)
				error(EXIT_FAILURE, errno, "select failed");
			continue;
		}
		else if (fd == 1) 
		{
			// printf("Socket ready to read\n"); // DEBUG
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
				if (rcv_seq <= app->sequence && rcv_seq >= 0) // accept all packets
					process_packet(bytes, app, rcv_seq);
				else
				{
					assert (rcv_seq < 0);
					printf("Error\n"); // TODO: remove
				}
			}
		}
		else
		{
			app->sequence++;
			send_echo(app);
			gettimeofday(&last, NULL);
		}
	}
	clean_up();
	return (0);
}
