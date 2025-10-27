/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:56:46 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/27 12:06:27 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <errno.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <sys/time.h>
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

/***** PARSE *****/
void	parse_args(int ac, char **av, t_ft_ping *app);
void	parse_flag(char *flag, t_ft_ping *app, char *prog_name);

/***** UTILS *****/
void	print_bytes(uint8_t *bytes, int len);
void	print_usage(char *prog_name);
void	print_addr(struct sockaddr_in *addr);

#endif