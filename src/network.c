/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   network.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 09:41:33 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/03 10:32:54 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	init_socket(t_ft_ping *app)
{
	int				raw_socket;

	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_socket < 0)
	{
		perror("ft_ping");
		fprintf(stderr, "Lacking privilege for icmp socket.\n");
		exit (1);
	}
	set_socket_options(raw_socket);
	app->socket = raw_socket;
}

int	set_socket_options(int raw_socket)
{
	int	enable;

	enable = 1;
	if (setsockopt(raw_socket, SOL_SOCKET, SO_TIMESTAMP,
			&enable, sizeof(enable)) != 0)
	{
		fprintf(stderr, "ft_ping: setsockopt (SO_TIMESTAMP): %s\n",
			strerror(errno));
	}
	if (setsockopt(raw_socket, SOL_SOCKET, SO_BROADCAST,
			(char *)&enable, sizeof(enable)) != 0)
	{
		fprintf(stderr, "ft_ping: setsockopt (SO_BROADCAST): %s\n",
			strerror(errno));
	}
	if (g_ft_ping->options[TTL])
	{
		if (setsockopt(raw_socket, IPPROTO_IP, IP_TTL,
				&g_ft_ping->options[TTL], sizeof(g_ft_ping->options[TTL])) != 0)
			fprintf(stderr, "ft_ping: setsockopt (IP_TTL): %s\n",
				strerror(errno));
	}
	return (0);
}

int	resolv_hostname(t_ft_ping *app)
{
	int					status;
	struct addrinfo		hints;
	struct sockaddr_in	*dest_addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	status = getaddrinfo(app->hostname, NULL, &hints, &app->res);
	if (status != 0)
		return (status);
	dest_addr = (struct sockaddr_in *)app->res->ai_addr;
	inet_ntop(app->res->ai_family, &(dest_addr->sin_addr), app->ip_str,
			sizeof(app->ip_str));
	app->dest_addr = *(struct sockaddr_in *)app->res->ai_addr;
	return (status);
}

void	setup_destination(t_ft_ping *app)
{
	int	status;
	
	status = resolv_hostname(app);
	if (status != 0)
	{
		if (status == EAI_SYSTEM)
			perror("ft_ping: getaddrinfo");
		else
			fprintf(stderr, "ft_ping: getaddrinfo: %s\n", gai_strerror(status));
		exit (1);
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
				ping_success(ip_header, app, rcv_seq);
			break ;
		case ICMP_ECHO:
			break ; // Ignore our own packet
		default:
			print_icmp_error(ip_header, icmp_header, bytes, app);
			break ;		
	}
}
