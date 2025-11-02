/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:58:13 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/02 11:05:11 by tfregni          ###   ########.fr       */
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