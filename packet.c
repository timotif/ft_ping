/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:48:54 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/29 16:24:04 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"


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
		sum += *(uint8_t *)data;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

void	process_packet(int bytes, t_ft_ping *app)
{
	int				offset;
	t_ip_header		*ip_header;
	t_icmp_header	*icmp_header;

	ip_header = (t_ip_header *) app->recvbuffer;
	offset = ip_header->ihl * 4;
	icmp_header = (struct icmphdr *)(app->recvbuffer + offset);
	if (icmp_header->type == ICMP_ECHOREPLY)
	{
		if (ntohs(icmp_header->un.echo.id) == app->pid)
			ping_success(ip_header, icmp_header, app);
	}
	else
		ping_fail(ip_header, icmp_header, bytes, app);
}


int	send_packet(int sock, uint8_t *sendbuffer, struct sockaddr_in *addr)
{
	int	bytes;

	bytes = sendto(sock, sendbuffer, PACKET_SIZE, 
			0, (struct sockaddr *)addr, sizeof(*addr));
	if (bytes < 0)
		perror("send packet");
	return (bytes);
}

int	receive_packet(int sock, uint8_t *recvbuffer, size_t bufsize, 
		struct sockaddr_in *reply_addr, uint16_t sequence)
{
	int			bytes;
	socklen_t	reply_addr_len;

	while (1)
	{
		reply_addr_len = sizeof(*reply_addr);
		memset(reply_addr, 0, reply_addr_len);
		bytes = recvfrom(sock, recvbuffer, bufsize, 0, 
				(struct sockaddr *) reply_addr, &reply_addr_len);
		if (sequence == buffer_get_sequence(recvbuffer, bufsize) || bytes < 0)
			break ;
	}
	return (bytes);
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
	packet->un.echo.id = htons(pid);
	packet->un.echo.sequence = htons(seq);
	memcpy(sendbuffer + sizeof(t_icmp_header), payload, payload_len);
	assert(packet->checksum == 0);
	packet->checksum = calculate_checksum((uint16_t*) packet, packet_size);
}