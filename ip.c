/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ip.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:57:23 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/01 14:38:12 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

char	*ip_get_source_addr(t_ip_header *ip_header)
{
	struct in_addr addr;

	addr.s_addr = ip_header->saddr;
	return (inet_ntoa(addr));
}

int	ip_is_valid(uint8_t *packet, size_t len)
{
	t_ip_header	*ip_header;

	if (!packet || len < 20)
		return (0);
	ip_header = (t_ip_header *) packet;
	if (ip_header->ihl < 5 || len < ip_header->ihl * 4)
		return (0);
	return (1);
}