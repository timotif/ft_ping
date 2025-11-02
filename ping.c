/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:55:07 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/02 13:07:27 by tfregni          ###   ########.fr       */
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

void	print_echo(int psize, t_ip_header *ip_header, int rcv_seq,
	long long time, int dup)
{
	/* 64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.022 ms */
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%lld.%03lld ms",
		psize, ip_get_source_addr(ip_header),
		rcv_seq,
		ip_header->ttl, time / 1000, time % 1000);
	if (dup)
		printf(" (DUP!)");
	printf("\n");
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
	print_echo(PACKET_SIZE, ip_header, rcv_seq, time, dup);
}

uint8_t	*extract_embedded_packet(uint8_t *error_packet, int *embedded_len)
{
	t_ip_header	*ip;
	int			hlen;

	ip = (t_ip_header *)error_packet;
	hlen = ip->ihl << 2;
	*embedded_len = *embedded_len - hlen - ICMP_HEADER_SIZE;
	return (error_packet + hlen + ICMP_HEADER_SIZE);
}

/* 
Handle ICMP error messages
Example: 56 bytes from 192.168.1.1: icmp_seq=3 Destination Host Unreachable
*/
void	ping_fail(t_ip_header *ip_header, t_icmp_header *icmp_header, 
		int bytes, t_ft_ping *app)
{
	int	hlen, datalen, embedded_len;
	
	hlen = ip_header->ihl << 2;
	datalen = bytes - hlen;
	// Bytes are the ICMP packet size, not the full IP packet size
	printf("%d bytes from %s: icmp_seq=%d ", datalen,
			ip_get_source_addr(ip_header), icmp_get_sequence(icmp_header));
	switch (icmp_header->type)
	{
		case ICMP_DEST_UNREACH:
			switch (icmp_header->code)
			{
				case (ICMP_HOST_UNREACH):
					printf("Destination Host Unreachable\n");
					break;
				case (ICMP_NET_UNREACH):
					printf("Destination Net Unreachable\n");
					break;
				case (ICMP_PROT_UNREACH):
					printf("Destination Protocol Unreachable\n");
					break;
				case (ICMP_PORT_UNREACH):
					printf("Destination Port Unreachable\n");
					break;
				case (ICMP_FRAG_NEEDED):
					printf("Fragmentation needed and DF set\n");
					break;
				case (ICMP_SR_FAILED):
					printf("Source Route Failed\n");
					break;
				case (ICMP_NET_UNKNOWN):
					printf("Network Unknown\n");
					break;
				case (ICMP_HOST_UNKNOWN):
					printf("Host Unknown\n");
					break;
				case (ICMP_HOST_ISOLATED):
					printf("Host Isolated\n");
					break;
				case (ICMP_NET_UNR_TOS):
					printf("Destination Network Unreachable At This TOS\n");
					break;
				case (ICMP_HOST_UNR_TOS):
					printf("Destination Host Unreachable At This TOS\n");
					break;
				default:
					printf("Destination Unreachable\n");
					break;
			}
			break;
		case (ICMP_REDIRECT):
			switch (icmp_header->code)
			{
				case (ICMP_REDIR_NET):
					printf("Redirect Network\n");
					break;
				case (ICMP_REDIR_HOST):
					printf("Redirect Host\n");
					break;
				case (ICMP_REDIR_NETTOS):
					printf("Redirect Type of Service and Network\n");
					break;
				case (ICMP_REDIR_HOSTTOS):
					printf("Redirect Type of Service and Host\n");
					break;
				default:
					printf("Redirect Message\n");
					break;
			}
			break;
		case (ICMP_TIME_EXCEEDED):
			switch (icmp_header->code)
			{
				case (ICMP_EXC_TTL):
					printf("Time to live exceeded\n");
					break;
				case (ICMP_EXC_FRAGTIME):
					printf("Frag reassembly time exceeded\n");
					break;
				default:
					printf("Time Exceeded\n");
					break;
			}
			break;
		default:
			fprintf(stderr, "Unknown Code: %d\n", icmp_header->code);
			break;
	}
	if (app->flags[VERBOSE])
	{
		embedded_len = bytes;
		// Since it's an error, dump the embedded ip message
		packet_dump(extract_embedded_packet((uint8_t *)ip_header, &embedded_len), embedded_len);
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
	long long	sec, usec, carry;

	sec = (long long)t->tv_sec;
	usec = (long long)t->tv_usec;
	carry = usec / 1000000LL;
	usec -= carry * 1000000LL;
	sec += carry;

	/* if usec negative after division adjust one more second */
	if (usec < 0)
	{
		usec += 1000000LL;
		sec -= 1;
	}

	t->tv_sec = (time_t)sec;
	t->tv_usec = (suseconds_t)usec;
}

/* Initalize timing structures for custom interval in ms */
void	initialize_timing(uint32_t interval_ms, struct timeval *interval,
		struct timeval *last, struct timeval *resp_time)
{
	memset(interval, 0, sizeof(*interval));
	memset(last, 0, sizeof(*last));
	memset(resp_time, 0, sizeof(*resp_time));
	interval->tv_sec = interval_ms / 1000;
	interval->tv_usec = (interval_ms % 1000) * 1000;
	gettimeofday(last, NULL);	
}

/* Calculate time remaining until next packet should be sent.
Returns time from now until (last + interval). */
void	calculate_timeout_remaining(struct timeval *timeout,
		const struct timeval *last, const struct timeval *interval)
{	
	struct timeval	now;

	gettimeofday(&now, NULL);
	timeout->tv_sec = last->tv_sec + interval->tv_sec - now.tv_sec;
	timeout->tv_usec = last->tv_usec + interval->tv_usec - now.tv_usec;
	normalize_timeval(timeout);
	// Clamp negative values to 0
	if (timeout->tv_sec < 0)
		timeout->tv_sec = timeout->tv_usec = 0;
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
		if (rcv_seq <= app->sequence && rcv_seq >= 0) // accept all packets
			process_packet(bytes, app, rcv_seq);
	}
}

void	handle_select_error(void)
{
	if (errno != EINTR)
		error(EXIT_FAILURE, errno, "select failed");
}

void	send_next_packet(t_ft_ping *app, struct timeval *last)
{
	app->sequence++;
	send_echo(app);
	gettimeofday(last, NULL);
}

int	ping_loop(t_ft_ping *app)
{
	struct timeval	interval, last, resp_time;
	fd_set			fdset;
	t_wait_result	wait_result;

	initialize_timing(INTERVAL, &interval, &last, &resp_time);
	app->sequence = 0;
	send_echo(app);
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
