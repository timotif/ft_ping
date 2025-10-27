/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:54:32 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/27 12:08:34 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"


int	init_socket(void)
{
	int	raw_socket;

	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_socket <= 0)
	{
		perror("socket");
		fprintf(stderr, "Hint: raw sockets require root privileges\n");
		exit (1);
	}
	// TODO: implement timeout
	return (raw_socket);
}

void	setup_destination(struct sockaddr_in *dest, uint32_t dest_ip)
{
	memset(dest, 0, sizeof(*dest));
	dest->sin_family = AF_INET;
	dest->sin_addr.s_addr = dest_ip;
}

uint32_t	calculate_checksum(uint16_t *data, uint32_t len)
{
	register long	sum;

	sum = 0;
	while (len > 1)
	{
		sum += *data++;
		len -= 2;
	}
	if (len > 0)
		sum += *(uint8_t*)data;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

void	prepare_packet(void *payload, size_t payload_len, uint8_t *sendbuffer, 
						int seq)
{
	t_icmp_header	*packet;
	size_t			packet_size;
	
	packet_size = PAYLOAD_SIZE + sizeof(t_icmp_header);
	memset(sendbuffer, 0, packet_size);
	packet = (t_icmp_header*) sendbuffer;
	packet->type = ICMP_ECHO; // 8
	packet->code = 0;
	packet->echo_id = getpid() & 0xffff; // masked to fit 16 bits
	packet->echo_sequence = seq;
	memcpy(sendbuffer + sizeof(t_icmp_header), payload, payload_len);
	packet->checksum = 0;
	packet->checksum = calculate_checksum((uint16_t*) packet, packet_size);
}

int	ping_loop(int sock, struct sockaddr_in *addr, t_ft_ping *app)
{
	uint8_t	sendbuffer[PAYLOAD_SIZE + sizeof(t_icmp_header)];
	char	payload[PAYLOAD_SIZE];
	struct timeval	start;
	// struct timeval	end; 
	
	
	while (1)
	{
		// Prepare packet
		memset(payload, 0x42, PAYLOAD_SIZE);
		prepare_packet(payload, PAYLOAD_SIZE, sendbuffer, 0);
		// print_bytes(sendbuffer, PAYLOAD_SIZE + sizeof(t_icmp_header));
		gettimeofday(&start, NULL);
	}
	(void) sock;
	(void) addr;
	(void) app;
	return (0);
	// Time snapshot
	// Send packet
	// Receive packet
	// Check it
	// Time end
}

int	main(int ac, char **av)
{
	uint32_t			dest_ip;
	struct sockaddr_in	dest;
	int					raw_socket;
	t_ft_ping			app;

	memset(&app, 0, sizeof(app));
	parse_args(ac, av, &app);
	// TODO: resolve through DNS
	if (inet_pton(AF_INET, av[1], &dest_ip) != 1)
	{
		fprintf(stderr, "Invalid address: %s\n", av[1]);
		return (1);
	}
	setup_destination(&dest, dest_ip);
	raw_socket = init_socket();
	// print_addr(&dest);
	printf("raw_socket: %d\n", raw_socket);
	printf("PING %s (%s): %ld data bytes\n",
		av[1],
		inet_ntoa(dest.sin_addr),
		PAYLOAD_SIZE + sizeof(t_icmp_header)
		);
	return (ping_loop(raw_socket, &dest, &app));
}
