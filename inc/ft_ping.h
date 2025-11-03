/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:56:46 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/03 10:21:14 by tfregni          ###   ########.fr       */
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
# include <netdb.h>
# include <sys/time.h>
# include <stdbool.h>
# include <signal.h>
# include <assert.h>
# include <math.h>
# include <sys/select.h>
# include <ctype.h>
# include <getopt.h>

# define ICMP_HEADER_SIZE 8
# define PAYLOAD_SIZE (PACKET_SIZE - ICMP_HEADER_SIZE)
# define PACKET_SIZE 64
# define MAX_IP_HEADER_SIZE 60
# define RECV_BUFFER_SIZE (PACKET_SIZE + MAX_IP_HEADER_SIZE)
# define INTERVAL_MS 1000
# define DUP_TABLE_SIZE 8192

typedef struct icmphdr	t_icmp_header;
typedef struct iphdr	t_ip_header;

enum	e_stat
{
	MIN,
	AVG,
	MAX,
	STDDEV
};

typedef enum e_wait_result
{
	WAIT_ERROR = -1,
	WAIT_TIMEOUT = 0,
	WAIT_READY = 1
}	t_wait_result;

enum	e_options
{
	VERBOSE,
	COUNT,
	INTERVAL,
	QUIET,
	TIMEOUT,
	TTL,
	USAGE,
	FLAGS_COUNT,
	ONLY_LONG = 255
};

// Application state - tracks metadata, not the headers themselves
typedef struct s_ft_ping
{
	volatile sig_atomic_t	stop; /* Written atomically (an int can be compiled in more assembly instructions that might be interrupted in the middle)*/
	uint16_t				options[FLAGS_COUNT]; // allocates for the amount of flags I implemented
	const char				*hostname;
	char					ip_str[INET_ADDRSTRLEN];
	int						socket;
	struct addrinfo			*res;	// Linked list of addrinfo structs from getaddrinfo
	uint16_t				pid;           				// process ID for echo_id
	uint16_t				sequence;      				// current sequence number
	uint8_t					rcv_map[DUP_TABLE_SIZE];	// bitmap to track duplicates
	size_t					packet_size;
	int						sent_packets;
	int						rcv_packets;
	int						dup_packets;
	struct timeval			start;
	struct timeval			end;
	uint8_t					sendbuffer[PACKET_SIZE];  		// ICMP header + payload
	uint8_t					recvbuffer[RECV_BUFFER_SIZE]; 	// received packet
	struct sockaddr_in		dest_addr;              		// destination address
	struct sockaddr_in		reply_addr;             		// address from last reply
	double					variance_m2;					// For the Welford algorithm
	long long				stats[4];
}	t_ft_ping;

/***** GLOBAL *****/
extern t_ft_ping	*g_ft_ping;

/***** CLEANUP & SIGNALS *****/
void	interrupt(int signum);
void	clean_up();

/***** SETUP *****/
void	setup_destination(t_ft_ping *app);
int		resolv_hostname(t_ft_ping *app);

/***** PRINT *****/
void	print_start_message(t_ft_ping *app);
void	print_echo(int psize, t_ip_header *ip_header, int rcv_seq,
	long long time, int dup);
void	packet_dump(uint8_t *bytes, size_t len);
void	print_help(char *prog_name);
void	print_usage(char *prog_name);
void	print_credits();
void	print_exit_message(t_ft_ping *app);

/***** PARSE *****/
void	parse_args(int ac, char **av, t_ft_ping *app);

/***** SOCKET *****/
void	init_socket(t_ft_ping *app);
int		set_socket_options(int raw_socket);

/***** TIME *****/
void	normalize_timeval(struct timeval *t);
void	initialize_timing(uint32_t interval_ms, struct timeval *interval,
		struct timeval *last, struct timeval *resp_time);
void	calculate_timeout_remaining(struct timeval *timeout,
		const struct timeval *last, const struct timeval *interval);

/***** UTILS *****/
long long	elapsed_time(struct timeval start, struct timeval end);

/***** PACKET *****/
void		prepare_echo_request_packet(void *payload, uint8_t *sendbuffer,
			int seq, pid_t pid);
uint32_t	calculate_checksum(uint16_t *data, uint32_t len);
int			send_packet(int sock, uint8_t *sendbuffer,
			struct sockaddr_in *addr);
int			receive_packet(int sock, uint8_t *recvbuffer, size_t bufsize,
			struct sockaddr_in *reply_addr, struct timeval *kernel_time);
void		process_packet(int bytes, t_ft_ping *app, int rcv_seq);

/***** PING *****/
void	print_icmp_error(t_ip_header *ip_header, t_icmp_header *icmp_header, 
			int bytes, t_ft_ping *app);
void	ping_success(t_ip_header *ip_header, t_ft_ping *app, int rcv_seq);
int		ping_loop(t_ft_ping *app);
int		ping_timeout(struct timeval *start_time, int timeout);

/***** IP *****/
char	*ip_get_source_addr(t_ip_header *ip_header);
int		ip_is_valid(uint8_t *packet, size_t len);

/***** ICMP *****/
uint16_t		icmp_get_sequence(t_icmp_header *icmp_header);
int16_t			buffer_get_sequence(uint8_t *buffer, size_t len);
int				verify_checksum(uint16_t *data, uint32_t len);
uint8_t			*extract_embedded_packet(uint8_t *error_packet,
			int *embedded_len);
t_icmp_header	*icmp_get_header(uint8_t *buffer, size_t len);

/***** BITMAP ****/
void	bitmap_set(uint8_t *bitmap, uint16_t n);
int		bitmap_test(uint8_t *bitmap, uint16_t n);
void	bitmap_clear(uint8_t *bitmap, uint16_t n);

/***** DEBUG *****/
void		print_bytes(uint8_t *bytes, size_t len, char *header);
void		print_icmp(uint8_t *bytes, size_t len);
void		print_ip(uint8_t *byes, size_t len);
#endif