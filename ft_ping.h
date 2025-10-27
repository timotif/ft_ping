/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:56:46 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/27 19:12:26 by tfregni          ###   ########.fr       */
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
# include <netinet/ip.h>
# include <sys/time.h>
# include <stdbool.h>
# include <assert.h>

# define PAYLOAD_SIZE PACKET_SIZE - 8
# define PACKET_SIZE 64

typedef struct icmphdr	t_icmp_header;
typedef struct iphdr	t_ip_header;

// Application state - tracks metadata, not the headers themselves
typedef struct s_ft_ping
{
	bool				verbose;
	const char			*dest;
	int					socket;
	uint16_t			pid;           // process ID for echo_id
	uint16_t			sequence;      // current sequence number
	int					sent_packets;
	int					rcv_packets;
	struct timeval		start;
	struct timeval		end;
	uint8_t				sendbuffer[PACKET_SIZE];  		// ICMP header + payload
	uint8_t				recvbuffer[PACKET_SIZE + 20]; 	// received packet
	struct sockaddr_in	dest_addr;              		// destination address
	struct sockaddr_in	reply_addr;             		// address from last reply
}	t_ft_ping;

/***** PARSE *****/
void	parse_args(int ac, char **av, t_ft_ping *app);
void	parse_flag(char *flag, t_ft_ping *app, char *prog_name);

/***** UTILS *****/
void	print_bytes(uint8_t *bytes, int len);
void	print_icmp(uint8_t *bytes, int len);
void	print_usage(char *prog_name);
void	print_addr(struct sockaddr_in *addr);
time_t	elapsed_time(struct timeval start, struct timeval end);

#endif