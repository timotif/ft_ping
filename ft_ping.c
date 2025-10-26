/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:54:32 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/26 19:23:13 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static void	print_usage(char *prog_name)
{
	fprintf(stderr, "Usage:\n\tsudo %s [-v] destination\n", prog_name);
}

void	parse_flag(char *flag, t_ft_ping *app, char *prog_name)
{
	flag++; // consume '-'
	if (!*flag)
	{
		fprintf(stderr, "Bad flag: illegal space\n");
		exit (1);
	}
	switch (*flag)
	{
		case 'v':
			app->verbose = true;
			break;
		case '?':
			print_usage(prog_name);
			exit (0);
		default:
			fprintf(stderr, "%s: -%c: Unrecognized option\n", prog_name, *flag);
			print_usage(prog_name);
			exit (1);
	}
}

void	parse_args(int ac, char **av, t_ft_ping *app)
{
	int		i;

	if (ac < 2)
	{
		print_usage(av[0]);
		exit (1);
	}
	i = 0;
	while (++i < ac)
	{
		if (av[i] && av[i][0])
		{
			if (av[i][0] == '-')
				parse_flag(av[i], app, av[0]);
			else
			{
				if (!app->dest)
					app->dest = av[i];
			}
		}
	}
	if (!app->dest)
	{
		print_usage(av[0]);
		exit (1);
	}
}

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
	return (raw_socket);
}

void	print_addr(struct sockaddr_in *addr)
{
	printf("%s\n", inet_ntoa(addr->sin_addr));
}

void	setup_destination(struct sockaddr_in *dest, uint32_t dest_ip)
{
	memset(dest, 0, sizeof(*dest));
	dest->sin_family = AF_INET;
	dest->sin_addr.s_addr = dest_ip;
}

int	ping_loop(int sock, struct sockaddr_in *addr)
{
	(void) sock;
	(void) addr;
	return (0);
	// Prepare packet
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
	printf("PING %s (%s) %d bytes of data.\n",
		av[1],
		inet_ntoa(dest.sin_addr),
		PAYLOAD_SIZE
		);
	return (ping_loop(raw_socket, &dest));
}
