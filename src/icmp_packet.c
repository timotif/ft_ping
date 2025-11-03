/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp_packet.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:58:13 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/03 09:57:47 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/* Returns the sequence number from an icmp packet checking its type */
uint16_t	icmp_get_sequence(t_icmp_header *icmp_header)
{
	struct icmp		*icmp_bsd;
	struct ip		*ip;
	int				ip_hlen;
	struct icmp		*icmp;

	if (icmp_header->type == ICMP_ECHOREPLY || icmp_header->type == ICMP_ECHO)
		return (ntohs(icmp_header->un.echo.sequence));
	icmp_bsd = (struct icmp *) icmp_header;
	ip = &icmp_bsd->icmp_ip;
	ip_hlen = ip->ip_hl << 2;
	icmp = (struct icmp *)((uint8_t *)ip + ip_hlen);
	return (ntohs(icmp->icmp_seq));
}

int16_t	buffer_get_sequence(uint8_t *buffer, size_t len)
{
	t_icmp_header	*icmp_header;

	icmp_header = icmp_get_header(buffer, len);
	if (!icmp_header)
		return (-1);
	return ((int16_t)icmp_get_sequence(icmp_header));
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

uint8_t	*extract_embedded_packet(uint8_t *error_packet, int *embedded_len)
{
	t_ip_header	*ip;
	int			hlen;

	ip = (t_ip_header *)error_packet;
	hlen = ip->ihl << 2;
	*embedded_len = *embedded_len - hlen - ICMP_HEADER_SIZE;
	return (error_packet + hlen + ICMP_HEADER_SIZE);
}

void	prepare_echo_request_packet(void *payload, 
		uint8_t *sendbuffer, int seq, pid_t pid)
{
	t_icmp_header	*packet;
	size_t			packet_size;
	
	packet_size = g_ft_ping->packet_size;
	memset(sendbuffer, 0, packet_size);
	packet = (t_icmp_header*) sendbuffer;
	packet->type = ICMP_ECHO; // 8
	packet->code = 0;
	packet->un.echo.id = htons(pid);
	packet->un.echo.sequence = htons(seq);
	memcpy(sendbuffer + sizeof(t_icmp_header), payload, packet_size - ICMP_HEADER_SIZE);
	assert(packet->checksum == 0);
	packet->checksum = calculate_checksum((uint16_t*) packet, packet_size);
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
		sum += *(uint8_t *)data;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

int	verify_checksum(uint16_t *data, uint32_t len)
{
	uint32_t		checksum_check;
	uint32_t		checksum_rcv;
	t_icmp_header	*icmp_header;
	
	icmp_header = (t_icmp_header *)data;
	checksum_rcv = icmp_header->checksum & 0xffff;
	icmp_header->checksum = 0;
	checksum_check = calculate_checksum((uint16_t *)icmp_header, len) & 0xffff;
	icmp_header->checksum = checksum_rcv;
	return (checksum_rcv == checksum_check);
}