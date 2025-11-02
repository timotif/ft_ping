/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:58:49 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/02 09:12:50 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	print_bytes(uint8_t *bytes, size_t len, char *header)
{
	size_t	i;

	if (header)
		printf("\n========== %s ==========\n", header);
	printf("Total bytes: %ld\n", len);
	printf("Hex dump: ");
	i = 0;
	while (i < len){
		if (i != 0 && i % 16 == 0)
			printf("\n          ");
		printf("%02x ", (uint8_t) bytes[i++]);
	}
	printf("\n");
}

/*
Example:
92 bytes from tfregni-ryzen (10.0.1.50): Destination Host Unreachable
IP Hdr Dump:
 4500 0054 257f 4000 4001 fdfa 0a00 0132 0a00 01fe 
Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data
 4  5  00 0054 257f   2 0000  40  01 fdfa 10.0.1.50  10.0.1.254 
ICMP: type 8, code 0, size 64, id 0x8a0e, seq 0x0000
*/
void	packet_dump(uint8_t *bytes, size_t len)
{
	t_ip_header	*ip;
	size_t		header_len;
	char		source_addr[INET_ADDRSTRLEN], dest_addr[INET_ADDRSTRLEN];

	ip = (t_ip_header *)bytes;
	header_len = ip->ihl << 2;
	printf("IP Hdr Dump:\n");
	for (size_t i = 0; i < len && i < header_len; i++)
		printf("%02x%s", bytes[i], (i % 2 == 0)? " ": ""); // Group bytes by 2
	printf("\n");
	uint16_t frag_off = ntohs(ip->frag_off);
	inet_ntop(AF_INET, &ip->saddr, source_addr, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &ip->daddr, dest_addr, INET_ADDRSTRLEN);
	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
	printf(" %01x  %01x  %02x %04x ", ip->version, ip->ihl, ip->tos, ntohs(ip->tot_len));
	printf("%04x   %01x %04x  ", ntohs(ip->id), (frag_off >> 13) & 0x7, frag_off & 0x1fff);
	printf("%02x  %02x %04x ", ip->ttl, ip->protocol, ntohs(ip->check));
	printf("%s  %s\n", source_addr, dest_addr);
	bytes += header_len; // point to options
	if (header_len > sizeof(*ip))
	{
		while (header_len-- > sizeof(*ip))
			printf("%02x", *bytes++);
	}
	if (ip->protocol != 1) // Supporting only ICMP for the moment
	{
		fprintf(stderr, "unsupported protocol: %d\n", ip->protocol);
		return ;
	}
	t_icmp_header *icmp = (t_icmp_header *)bytes;
	printf("ICMP: ");
	printf("type %d, code %d, size %ld", 
		icmp->type, icmp->code, len - header_len);
	switch (icmp->type)
	{
		case (ICMP_ECHO):
			printf(", id 0x%04x, seq 0x%04x\n", ntohs(icmp->un.echo.id),
					ntohs(icmp->un.echo.sequence));
			break;
		case (ICMP_ECHOREPLY):
			printf("Echo Reply\n");
			break;
	}
}

void	print_ip(uint8_t *bytes, size_t len)
{
	t_ip_header		*ip;
	size_t			ip_header_len;

	print_bytes(bytes, len, "RAW PACKET");
	printf("\n========== IP HEADER ==========\n");
	ip = (t_ip_header *)bytes;
	ip_header_len = ip->ihl * 4;
	printf("Version:        %d\n", ip->version);
	printf("IHL:            %d (%ld bytes)\n", ip->ihl, ip_header_len);
	printf("TOS:            0x%02x\n", ip->tos);
	printf("Total Length:   %d (wire: %02x %02x)\n",
		ntohs(ip->tot_len), bytes[2], bytes[3]);
	printf("ID:             %d (wire: %02x %02x)\n",
		ntohs(ip->id), bytes[4], bytes[5]);
	printf("Frag offset:    %d (wire: %02x %02x)\n",
		ntohs(ip->frag_off), bytes[6], bytes[7]);
	printf("TTL:            %d\n", ip->ttl);
	printf("Protocol:       %d %s\n", ip->protocol,
		ip->protocol == IPPROTO_ICMP ? "(ICMP)" : "");
	printf("Checksum:       0x%04x (wire: %02x %02x)\n",
		ntohs(ip->check), bytes[10], bytes[11]);
	printf("Source IP:      %s (wire: %02x %02x %02x %02x)\n",
		inet_ntoa(*(struct in_addr *)&ip->saddr),
		bytes[12], bytes[13], bytes[14], bytes[15]);
	printf("Dest IP:        %s (wire: %02x %02x %02x %02x)\n",
		inet_ntoa(*(struct in_addr *)&ip->daddr),
		bytes[16], bytes[17], bytes[18], bytes[19]);
	if (len <= ip_header_len)
		return ;
	switch (ip->protocol)
	{
		case IPPROTO_ICMP:
			print_icmp(bytes + ip_header_len, len - ip_header_len);
			break;
		default:	/* Handle more protocols here */
			fprintf(stderr,"Unrecognized protocol: %d\n", ip->protocol);
	}
	printf("=====================================\n\n");
}

void	print_icmp(uint8_t *bytes, size_t len)
{
	t_icmp_header	*icmp;
	size_t			i;

	printf("\n========== ICMP HEADER ==========\n");
	icmp = (t_icmp_header *)(bytes);
	printf("Type:           %d\n", icmp->type);
	printf("Code:           %d\n", icmp->code);
	printf("Checksum:       0x%04x (wire: %02x %02x)\n",
		ntohs(icmp->checksum), bytes[2],
		bytes[3]);
	printf("ID:             %d (wire: %02x %02x)\n",
		ntohs(icmp->un.echo.id), bytes[4],
		bytes[5]);
	printf("Sequence:       %d (wire: %02x %02x)\n",
		ntohs(icmp->un.echo.sequence), bytes[6],
		bytes[7]);

	printf("\nICMP Payload (%ld bytes):\n",
		len - 8);
	printf("          ");
	for (i = 8; i < len && i < 24; i++)
		printf("%02x ", bytes[i]);
	if (len > 24)
		printf("...");
	printf("\n");
}

void	print_usage(char *prog_name)
{
	fprintf(stderr, "Usage:\n\tsudo %s [-v] destination\n", prog_name);
}

void	print_addr(struct sockaddr_in *addr)
{
	// TODO: update with flags when implement
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