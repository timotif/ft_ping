/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:48:54 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/01 15:15:38 by tfregni          ###   ########.fr       */
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

void	process_packet(int bytes, t_ft_ping *app, int rcv_seq)
{
	int				offset;
	t_ip_header		*ip_header;
	t_icmp_header	*icmp_header;

	if (!ip_is_valid(app->recvbuffer, bytes))
	{
		printf("Invalid packet\n"); // TODO: remove
		return ;
	}
	ip_header = (t_ip_header *) app->recvbuffer;
	offset = ip_header->ihl * 4;
	icmp_header = (struct icmphdr *)(app->recvbuffer + offset);
	if (ntohs(icmp_header->un.echo.id) == app->pid)
	{
		if (icmp_header->type == ICMP_ECHOREPLY)
			ping_success(ip_header, icmp_header, app, rcv_seq);
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
	
	packet_size = PACKET_SIZE;
	memset(sendbuffer, 0, packet_size);
	packet = (t_icmp_header*) sendbuffer;
	packet->type = ICMP_ECHO; // 8
	packet->code = 0;
	packet->un.echo.id = htons(pid);
	packet->un.echo.sequence = htons(seq);
	memcpy(sendbuffer + sizeof(t_icmp_header), payload, PAYLOAD_SIZE);
	assert(packet->checksum == 0);
	packet->checksum = calculate_checksum((uint16_t*) packet, packet_size);
}