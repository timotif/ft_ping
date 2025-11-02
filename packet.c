/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:48:54 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/02 21:56:33 by tfregni          ###   ########.fr       */
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

void	process_packet(int bytes, t_ft_ping *app, int rcv_seq)
{
	int				offset;
	t_ip_header		*ip_header;
	t_icmp_header	*icmp_header;

	if (!ip_is_valid(app->recvbuffer, bytes))
		return ;
	ip_header = (t_ip_header *) app->recvbuffer;
	offset = ip_header->ihl << 2;
	icmp_header = (struct icmphdr *)(app->recvbuffer + offset);
	if (!verify_checksum((uint16_t *)icmp_header, bytes - offset))
		fprintf(stderr, "checksum mismatch from %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
	switch (icmp_header->type)
	{
		case ICMP_ECHOREPLY:
			if (ntohs(icmp_header->un.echo.id) == app->pid)
				// packet_dump(app->recvbuffer, bytes); // DEBUG
				ping_success(ip_header, app, rcv_seq);
			break ;
		case ICMP_ECHO:
			break ; // Ignore our own packet
		default:
			ping_fail(ip_header, icmp_header, bytes, app);
			break ;		
	}
}


int	send_packet(int sock, uint8_t *sendbuffer, struct sockaddr_in *addr)
{
	int	bytes;

	bytes = sendto(sock, sendbuffer, g_ft_ping->packet_size, 
			0, (struct sockaddr *)addr, sizeof(*addr));
	if (bytes < 0)
		perror("send packet");
	return (bytes);
}

int	receive_packet(int sock, uint8_t *recvbuffer, size_t bufsize,
		struct sockaddr_in *reply_addr,	struct timeval *kernel_time)
{
	int				bytes;
	struct msghdr	msg;
	struct iovec	iov;
	char			control[1024];
	struct cmsghdr	*cmsg;

	memset(reply_addr, 0, sizeof(*reply_addr));
	memset(&msg, 0, sizeof(msg));
	memset(control, 0, sizeof(control));
	iov.iov_base = recvbuffer;
	iov.iov_len = bufsize;
	msg.msg_name = reply_addr;
	msg.msg_namelen = sizeof(*reply_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);
	bytes = recvmsg(sock, &msg, 0);
	if (bytes >= 0 && kernel_time)
	{
		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg))
		{
			if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMP)
				*kernel_time = *(struct timeval *)CMSG_DATA(cmsg);
		}
	}
	return (bytes);
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