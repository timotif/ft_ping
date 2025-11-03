/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   output_format.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 09:44:16 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/03 10:21:45 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	print_start_message(t_ft_ping *app)
{
	printf("PING %s (%s): %ld data bytes",
		app->hostname,
		app->ip_str,
		app->packet_size - ICMP_HEADER_SIZE
		);
	if (app->options[VERBOSE])
		printf(", id 0x%04x = %d", app->pid, app->pid);
	printf("\n");
}

void	print_echo(int psize, t_ip_header *ip_header, int rcv_seq,
	long long time, int dup)
{
	if (g_ft_ping->options[QUIET])
		return ;
	/* 64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.022 ms */
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%lld.%03lld ms",
		psize, ip_get_source_addr(ip_header),
		rcv_seq,
		ip_header->ttl, time / 1000, time % 1000);
	if (dup)
		printf(" (DUP!)");
	printf("\n");
}

/* 
Handle ICMP error messages
Example: 56 bytes from 192.168.1.1: icmp_seq=3 Destination Host Unreachable
*/
void	print_icmp_error(t_ip_header *ip_header, t_icmp_header *icmp_header, 
		int bytes, t_ft_ping *app)
{
	int	hlen, datalen, embedded_len;

	if (app->options[QUIET])
		return ;
	hlen = ip_header->ihl << 2;
	datalen = bytes - hlen;
	// Bytes are the ICMP packet size, not the full IP packet size
	printf("%d bytes from %s: ", datalen,
			ip_get_source_addr(ip_header));
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
	if (app->options[VERBOSE])
	{
		embedded_len = bytes;
		// Since it's an error, dump the embedded ip message
		packet_dump(extract_embedded_packet((uint8_t *)ip_header, &embedded_len), embedded_len);
	}
}

void	print_exit_message(t_ft_ping *app)
{
	float		loss;
	
	if (!app)
		return ;
	loss = 0.0;
	if (g_ft_ping->sent_packets > 0)
	loss = 100 - (g_ft_ping->rcv_packets * 100 / g_ft_ping->sent_packets);
	/* Example:	
	--- 1.1.1.1 ping statistics ---
	1 packets transmitted, 1 packets received, 0% packet loss */
	printf("--- %s ping statistics ---\n", g_ft_ping->hostname);
	printf("%d packets transmitted, %d packets received, ", g_ft_ping->sent_packets, g_ft_ping->rcv_packets);
	if (app->dup_packets)
		printf("+%d duplicates, ", app->dup_packets);
	printf("%.1f%% packet loss\n", loss);
	/* Example: 
	round-trip min/avg/max/stddev = 31.634/31.634/31.634/0.000 ms */
	printf("round-trip min/avg/max/stddev = %lld.%lld/%lld.%lld/%lld.%lld/%lld.%lld ms\n",
	app->stats[MIN] / 1000, app->stats[MIN] % 1000,
	app->stats[AVG] / 1000, app->stats[AVG] % 1000,
	app->stats[MAX] / 1000, app->stats[MAX] % 1000,
	app->stats[STDDEV] / 1000, app->stats[STDDEV] % 1000);
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
	size_t		ip_len, header_len;
	char		source_addr[INET_ADDRSTRLEN], dest_addr[INET_ADDRSTRLEN];

	ip = (t_ip_header *)bytes;
	header_len = ip->ihl << 2;
	ip_len = ntohs(ip->tot_len);
	printf("IP Hdr Dump:\n ");
	for (size_t i = 0; i < len && i < header_len; i++)
		printf("%02x%s", (uint16_t)bytes[i], (i % 2)? " ": ""); // Group bytes by 2
	printf("\n");
	uint16_t frag_off = ntohs(ip->frag_off);
	inet_ntop(AF_INET, &ip->saddr, source_addr, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &ip->daddr, dest_addr, INET_ADDRSTRLEN);
	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
	printf(" %01x  %01x  %02x %04lx ", ip->version, ip->ihl, ip->tos, ip_len);
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
		icmp->type, icmp->code, ip_len - header_len);
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

void	print_help(char *prog_name)
{
	printf("Usage: sudo %s [OPTION...] [HOST...]\n", prog_name);
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
	printf(" Options valid for all request types:\n\n");
	printf("  -c,	--count=NUMBER		after sending NUMBER packets\n");
	printf("  -i,	--interval=NUMBER	wait NUMBER seconds between sending each packet\n");
	printf("     	--ttl=N				specify N as time-to-live\n");
	printf("  -v,	--verbose			verbose output\n");
	printf("  -w,	--timeout=N			stop after N seconds\n");
	printf("\n");
	printf(" Options valid for --echo requests:\n\n");
	printf("  -q,	--quiet				quiet output\n");
	printf("\n");
	printf("  -?,	--help				give this help list\n");
	printf("     	--usage				give a short usage message\n");
	printf("  -V						print program version\n");
} // TODO: fix alignment

void	print_usage(char *prog_name)
{
	printf("Usage: sudo %s [-vq?V] [-c NUMBER] [-i NUMBER] [-w N] [--ttl=N] ", prog_name);
	printf("HOST ...\n");
}

void	print_credits()
{
	printf("FT_PING @42Berlin v0.1\n");
	printf("Copyright (C) 2025 - All rights reserved (but we're not that serious about it)\n");
	printf("License: WTFPL - Do What The F*** You Want To Public License\n");
	printf("This program is free software. You can redistribute it and/or modify it\n");
	printf("under the terms of the WTFPL, Version 2, as published by Sam Hocevar.\n");
	printf("See http://www.wtfpl.net/ for more details.\n\n");
	printf("NO WARRANTY: This software is provided \"as is\", without warranty of any kind.\n");
	printf("If it breaks, you get to keep both pieces. If it works, consider yourself lucky.\n");
	printf("The author is not responsible for any packets lost in the void, angry network\n");
	printf("administrators, or existential crises caused by pondering ICMP philosophy.\n\n");
	printf("Written by Timoti Fregni.\n\n");
}
