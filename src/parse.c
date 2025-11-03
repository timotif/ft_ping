/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 11:05:14 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/03 08:07:34 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/*
 * Parse integer with range validation for uint16_t storage
 * Returns parsed value on success, exits on error
 */
static uint16_t	parse_uint16(char *optarg, char *prog_name, char *opt_name,
				long min, long max)
{
	char	*endptr;
	long	value;

	errno = 0;
	value = strtol(optarg, &endptr, 10);
	if (errno == ERANGE || *endptr != '\0' || endptr == optarg)
	{
		fprintf(stderr, "%s: invalid %s value: %s\n", prog_name,
			opt_name, optarg);
		exit(1);
	}
	if (value < min || value > max)
	{
		fprintf(stderr, "%s: %s must be between %ld and %ld\n",
			prog_name, opt_name, min, max);
		exit(1);
	}
	return ((uint16_t)value);
}

/*
 * Parse floating point interval with range validation
 * Converts to milliseconds for uint16_t storage
 */
static uint16_t	parse_interval(char *optarg, char *prog_name)
{
	char	*endptr;
	double	value;

	errno = 0;
	value = strtod(optarg, &endptr);
	if (errno == ERANGE || *endptr != '\0' || endptr == optarg)
	{
		fprintf(stderr, "%s: invalid interval value: %s\n",
			prog_name, optarg);
		exit(1);
	}
	if (value < 0.2 || value > 65.535)
	{
		fprintf(stderr, "%s: interval must be between 0.2 and 65.535 seconds\n",
			prog_name);
		exit(1);
	}
	return ((uint16_t)round(value * 1000.0));
}

static struct option s_long_options[] = 
{
	{"count", 		required_argument,	0, 'c'},
	{"interval",	required_argument, 	0, 'i'},
	{"ttl",			required_argument,	0, TTL + ONLY_LONG}, // Options with only long value start from 256
	{"verbose",		no_argument,		0, 'v'},
	{"timeout", 	required_argument,	0, 'w'},
	{"quiet", 		no_argument,		0, 'q'},
	{"help", 		no_argument,		0, '?'},
	{"usage",		no_argument,		0, USAGE + ONLY_LONG},
	{0, 0, 0, 0}
};

/**
 * Parse command line arguments using POSIX getopt()
 * Option string "Vvc:i:qw:?":
 *   - 'V' = version (no argument)
 *   - 'v' = verbose flag (no argument)
 *   - 'c:' = count option (requires argument)
 *   - 'i:' = interval option (requires argument)
 *   - 'q' = quiet flag (no argument)
 *   - 'w:' = timeout option (requires argument)
 *   - 'ttl:' = ttl option (requires argument, long-only)
 *   - '?' = help flag (no argument)
 */
void	parse_args(int ac, char **av, t_ft_ping *app)
{
	int	opt;
	int	option_index;

	while ((opt = getopt_long(ac, av, "Vvc:i:qw:?",
			s_long_options, &option_index)) != -1)
	{
		if (opt == 'v')
			app->options[VERBOSE] = 1;
		else if (opt == 'V')
		{
			print_credits();
			exit(0);
		}
		else if (opt == 'c')
			app->options[COUNT] = parse_uint16(optarg, av[0], "count", 1, 65535);
		else if (opt == 'i')
			app->options[INTERVAL] = parse_interval(optarg, av[0]);
		else if (opt == 'q')
			app->options[QUIET] = 1;
		else if (opt == 'w')
			app->options[TIMEOUT] = parse_uint16(optarg, av[0], "timeout",
				1, 65535);
		else if (opt == TTL + ONLY_LONG)
			app->options[TTL] = parse_uint16(optarg, av[0], "ttl", 1, 255);
		else if (opt == USAGE + ONLY_LONG)
		{
			print_usage(av[0]);
			exit(0);
		}
		else if (opt == '?')
		{
			print_help(av[0]);
			exit(0);
		}
		else
		{
			fprintf(stderr, "%s: unexpected error in getopt\n", av[0]);
			exit(1);
		}
	}
	if (optind < ac)
		app->hostname = av[optind];
	else
	{
		fprintf(stderr, "%s: missing hostname\n", av[0]);
		print_help(av[0]);
		exit(1);
	}
	if (optind + 1 < ac)
	{
		fprintf(stderr, "%s: extra arguments after hostname\n", av[0]);
		exit(1);
	}
}
