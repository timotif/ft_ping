/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:58:49 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/28 16:38:01 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	print_bytes(uint8_t *bytes, int len, char *header)
{
	int	i;

	if (header)
		printf("\n========== %s ==========\n", header);
	printf("Total bytes: %d\n", len);
	printf("Hex dump: ");
	i = 0;
	while (i < len){
		if (i != 0 && i % 16 == 0)
			printf("\n          ");
		printf("%02x ", (uint8_t) bytes[i++]);
	}
	printf("\n");
}

void	print_ip(uint8_t *buffer, u_int32_t total_bytes)
{
	t_ip_header		*ip;
	t_icmp_header	*icmp;
	size_t			ip_header_len;
	size_t			i;

	print_bytes(buffer, total_bytes, "RAW PACKET");
	printf("\n\n========== IP HEADER ==========\n");
	ip = (t_ip_header *)buffer;
	ip_header_len = ip->ihl * 4;
	printf("Version:        %d\n", ip->version);
	printf("IHL:            %d (%ld bytes)\n", ip->ihl, ip_header_len);
	printf("TOS:            0x%02x\n", ip->tos);
	printf("Total Length:   %d (wire: %02x %02x)\n",
		ntohs(ip->tot_len), buffer[2], buffer[3]);
	printf("ID:             %d (wire: %02x %02x)\n",
		ntohs(ip->id), buffer[4], buffer[5]);
	printf("Frag offset:    %d (wire: %02x %02x)\n",
		ntohs(ip->frag_off), buffer[6], buffer[7]);
	printf("TTL:            %d\n", ip->ttl);
	printf("Protocol:       %d %s\n", ip->protocol,
		ip->protocol == IPPROTO_ICMP ? "(ICMP)" : "");
	printf("Checksum:       0x%04x (wire: %02x %02x)\n",
		ntohs(ip->check), buffer[10], buffer[11]);
	printf("Source IP:      %s (wire: %02x %02x %02x %02x)\n",
		inet_ntoa(*(struct in_addr *)&ip->saddr),
		buffer[12], buffer[13], buffer[14], buffer[15]);
	printf("Dest IP:        %s (wire: %02x %02x %02x %02x)\n",
		inet_ntoa(*(struct in_addr *)&ip->daddr),
		buffer[16], buffer[17], buffer[18], buffer[19]);

	if (ip->protocol == IPPROTO_ICMP && total_bytes > ip_header_len)
	{
		printf("\n========== ICMP HEADER ==========\n");
		icmp = (t_icmp_header *)(buffer + ip_header_len);
		printf("Type:           %d\n", icmp->type);
		printf("Code:           %d\n", icmp->code);
		printf("Checksum:       0x%04x (wire: %02x %02x)\n",
			ntohs(icmp->checksum), buffer[ip_header_len + 2],
			buffer[ip_header_len + 3]);
		printf("ID:             %d (wire: %02x %02x)\n",
			ntohs(icmp->un.echo.id), buffer[ip_header_len + 4],
			buffer[ip_header_len + 5]);
		printf("Sequence:       %d (wire: %02x %02x)\n",
			ntohs(icmp->un.echo.sequence), buffer[ip_header_len + 6],
			buffer[ip_header_len + 7]);

		printf("\nICMP Payload (%ld bytes):\n",
			total_bytes - ip_header_len - 8);
		printf("          ");
		for (i = ip_header_len + 8; i < total_bytes && i < ip_header_len + 24; i++)
			printf("%02x ", buffer[i]);
		if (total_bytes > ip_header_len + 24)
			printf("...");
		printf("\n");
	}
	printf("=====================================\n\n");
}

void	print_icmp(uint8_t *bytes, int len)
{
	int	i;

	i = 0;
	while (i < 8)
	{
		if (i == 0)
			printf("Type:\t");
		if (i == 1)
			printf("\nCode:\t");
		if (i == 2)
			printf("\nChksum:\t");
		if (i == 4)
			printf("\nRest:\t");
		printf("%02x", (uint8_t) bytes[i++]);
	}
	printf("\nPayload:\t");
	print_bytes(bytes + 8, len - 8, NULL);
}

void	print_usage(char *prog_name)
{
	fprintf(stderr, "Usage:\n\tsudo %s [-v] destination\n", prog_name);
}

void	print_addr(struct sockaddr_in *addr)
{
	printf("%s\n", inet_ntoa(addr->sin_addr));
}

/* Returns the elapsed time in microseconds */
long long	elapsed_time(struct timeval start, struct timeval end)
{
	long long	start_usec;
	long long	end_usec;

	start_usec = (long long)start.tv_sec * 1000000 + (long long)start.tv_usec;
	end_usec = (long long)end.tv_sec * 1000000 + (long long)end.tv_usec;
	return (end_usec - start_usec);
}