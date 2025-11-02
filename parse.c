/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 11:05:14 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/02 15:47:45 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/*
Returns a pointer to the first non-digit char or NULL if it's number  
 */
static char	*isnumber(char *s)
{
	if (!s || !*s)
		return (s);
	while (*s)
	{
		if (!isdigit(*s))
			return (s);
		s++;
	}
	return (NULL);
}

/**
Parse command line arguments using POSIX getopt()
Option string "vc:h":
  - 'v' = verbose flag (no argument)
  - 'c:' = count option (requires argument)
  - '?' = help flag (no argument)
 */
void	parse_args(int ac, char **av, t_ft_ping *app)
{
	int		opt;
	
	// getopt() processes options until it finds a non-option or reaches end
	while ((opt = getopt(ac, av, "Vvc:?")) != -1)
	{
		switch (opt)
		{
			case 'v':
			// Simple flag - just set it
			app->flags[VERBOSE] = 1;
			break;
			case 'c':
			// Option with argument - optarg points to the value
			if (isnumber(optarg))
			{
				fprintf(stderr, "%s: invalid value: %s near %s\n", av[0], 
					optarg, isnumber(optarg));
					exit(1);
				}
				app->flags[COUNT] = (uint16_t)atoi(optarg);
				break;
				case 'h':
				print_usage(av[0]);
				exit(0);
			case 'V':
				print_credits();
				exit(0);
			case '?':
				// getopt() automatically prints error for unknown options
				// and sets opt to '?'
				print_usage(av[0]);
				exit(0);

			default:
				// Should never reach here with proper option string
				fprintf(stderr, "%s: unexpected error in getopt\n", av[0]);
				exit(1);
		}
	}

	// After getopt(), optind points to first non-option argument
	// This should be the hostname
	if (optind < ac)
		app->hostname = av[optind];
	else
	{
		fprintf(stderr, "%s: missing hostname\n", av[0]);
		print_usage(av[0]);
		exit(1);
	}

	// Check if there are extra arguments after hostname
	if (optind + 1 < ac)
	{
		fprintf(stderr, "%s: extra arguments after hostname\n", av[0]);
		exit(1);
	}
}
