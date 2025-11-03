/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 16:54:32 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/03 09:56:42 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

t_ft_ping	*g_ft_ping = NULL;


void	clean_up()
{
	if (!g_ft_ping)
		return ;
	// Only print exit message if we actually started pinging (sent at least one packet)
	if (g_ft_ping->sent_packets > 0)
		print_exit_message(g_ft_ping);
	if (g_ft_ping->socket > 0)
		close(g_ft_ping->socket);
	if (g_ft_ping->res)
		freeaddrinfo(g_ft_ping->res);
	g_ft_ping = NULL;
}

void	interrupt(int signum)
{
	(void) signum; // Required by signal() but unused
	g_ft_ping->stop = 1;
}

static void	init_app(t_ft_ping *app)
{
	memset(app, 0, sizeof(*app));
	app->pid = getpid();
	app->socket = -1; // at 0 the cleanup might close stdin
	app->packet_size = PACKET_SIZE;
	app->options[INTERVAL] = INTERVAL_MS;
}

int	main(int ac, char **av)
{
	t_ft_ping			app;

	init_app(&app);
	g_ft_ping = &app;
	atexit(clean_up); // clean up when exit() is called
	parse_args(ac, av, &app);
	setup_destination(&app);
	init_socket(&app);
	print_start_message(&app);
	return (ping_loop(&app));
}
