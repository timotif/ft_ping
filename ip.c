/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ip.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 17:57:23 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/01 10:17:38 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

char	*ip_get_source_addr(t_ip_header *ip_header)
{
	struct in_addr addr;

	addr.s_addr = ip_header->saddr;
	return (inet_ntoa(addr));
}