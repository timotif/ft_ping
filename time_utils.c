/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 09:50:47 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/03 09:53:30 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/* Normalizes a timeval-like structure's microseconds field so the pair 
(tv_sec, tv_usec) represents the same total time but with tv_usec in the 
canonical range [0, 10^6)*/
void	normalize_timeval(struct timeval *t)
{
	long long	sec, usec, carry;

	sec = (long long)t->tv_sec;
	usec = (long long)t->tv_usec;
	carry = usec / 1000000LL;
	usec -= carry * 1000000LL;
	sec += carry;

	/* if usec negative after division adjust one more second */
	if (usec < 0)
	{
		usec += 1000000LL;
		sec -= 1;
	}

	t->tv_sec = (time_t)sec;
	t->tv_usec = (suseconds_t)usec;
}

/* Initalize timing structures for custom interval in ms */
void	initialize_timing(uint32_t interval_ms, struct timeval *interval,
		struct timeval *last, struct timeval *resp_time)
{
	memset(interval, 0, sizeof(*interval));
	memset(last, 0, sizeof(*last));
	memset(resp_time, 0, sizeof(*resp_time));
	interval->tv_sec = interval_ms / 1000;
	interval->tv_usec = (interval_ms % 1000) * 1000;
	gettimeofday(last, NULL);	
}

/* Calculate time remaining until next packet should be sent.
Returns time from now until (last + interval). */
void	calculate_timeout_remaining(struct timeval *timeout,
		const struct timeval *last, const struct timeval *interval)
{	
	struct timeval	now;

	gettimeofday(&now, NULL);
	timeout->tv_sec = last->tv_sec + interval->tv_sec - now.tv_sec;
	timeout->tv_usec = last->tv_usec + interval->tv_usec - now.tv_usec;
	normalize_timeval(timeout);
	// Clamp negative values to 0
	if (timeout->tv_sec < 0)
		timeout->tv_sec = timeout->tv_usec = 0;
}

/* Returns the elapsed time in microseconds */
long long	elapsed_time(struct timeval start, struct timeval end)
{
	long long	start_usec;
	long long	end_usec;

	start_usec = (long long)start.tv_sec * 1000000 + (long long)start.tv_usec;
	end_usec = (long long)end.tv_sec * 1000000 + (long long)end.tv_usec;
	return (end_usec - start_usec);
}