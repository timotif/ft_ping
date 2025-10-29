/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:56:46 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/29 18:41:56 by tfregni          ###   ########.fr       */
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
# include <signal.h>
# include <assert.h>

# define PAYLOAD_SIZE PACKET_SIZE - 8
# define PACKET_SIZE 64
# define SOCKET_TIMEOUT 1
# define MAX_IP_HEADER_SIZE 60
# define RECV_BUFFER_SIZE (PACKET_SIZE + MAX_IP_HEADER_SIZE)

typedef struct icmphdr	t_icmp_header;
typedef struct iphdr	t_ip_header;

enum	e_stat
{
	MIN,
	AVG,
	MAX,
	STDDEV
};

typedef enum	e_packet_type // TODO: maybe delete
{
	ICMP,
	IP,
	PAYLOAD
}	t_packet_type;

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
	uint8_t				recvbuffer[RECV_BUFFER_SIZE]; 	// received packet
	struct sockaddr_in	dest_addr;              		// destination address
	struct sockaddr_in	reply_addr;             		// address from last reply
	long long			stats[4];
}	t_ft_ping;

/***** GLOBAL *****/
extern t_ft_ping	*g_ft_ping;

/***** PARSE *****/
void	parse_args(int ac, char **av, t_ft_ping *app);
void	parse_flag(char *flag, t_ft_ping *app, char *prog_name);

/***** SOCKET *****/
int		init_socket(void);
int		set_socket_options(int raw_socket);

/***** UTILS *****/
void		print_bytes(uint8_t *bytes, size_t len, char *header);
void		print_icmp(uint8_t *bytes, size_t len);
void		print_ip(uint8_t *byes, size_t len);
void		print_usage(char *prog_name);
void		print_addr(struct sockaddr_in *addr);
long long	elapsed_time(struct timeval start, struct timeval end);

/***** PACKET *****/
void		prepare_echo_request_packet(void *payload, uint8_t *sendbuffer,
			int seq, pid_t pid);
uint32_t	calculate_checksum(uint16_t *data, uint32_t len);
int			send_packet(int sock, uint8_t *sendbuffer,
			struct sockaddr_in *addr);
int			receive_packet(int sock, uint8_t *recvbuffer, size_t bufsize,
			struct sockaddr_in *reply_addr, uint16_t sequence,
			struct timeval *kernel_time);
void		process_packet(int bytes, t_ft_ping *app);

/***** PING *****/
void	ping_fail(t_ip_header *ip_header, t_icmp_header *icmp_header, 
			int bytes, t_ft_ping *app);
void	ping_success(t_ip_header *ip_header, t_icmp_header *icmp_header, 
			t_ft_ping *app);
int		ping_loop(int sock, t_ft_ping *app);

/***** IP *****/
char	*ip_get_source_addr(t_ip_header *ip_header);

/***** ICMP *****/
uint16_t	icmp_get_sequence(t_icmp_header *icmp_header);

uint16_t	buffer_get_sequence(uint8_t *buffer, int len);
#endif