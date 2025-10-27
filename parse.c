/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 11:05:14 by tfregni           #+#    #+#             */
/*   Updated: 2025/10/27 11:05:44 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

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