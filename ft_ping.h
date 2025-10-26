/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:56:46 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/26 17:32:30 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <stdbool.h>

# define PAYLOAD_SIZE 56

// Simplified ICMP Header structure from /usr/include/linux/icmp.h
typedef struct s_icmp_header
{
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;
	uint16_t	echo_id;
	uint16_t	echo_sequence;
}	t_icmp_header;

typedef struct s_ft_ping
{
	bool			verbose;
	char			*dest;
	t_icmp_header	icmp;
}	t_ft_ping;
#endif