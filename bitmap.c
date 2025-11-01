/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bitmap.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/01 13:41:11 by tfregni           #+#    #+#             */
/*   Updated: 2025/11/01 14:29:00 by tfregni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	bitmap_set(uint8_t *bitmap, uint16_t n)
{
	int	index;
	int	offset;

	index = n >> 3;
	offset = n & 0x7;
	bitmap[index] |= (1 << offset);
}

int	bitmap_test(uint8_t *bitmap, uint16_t n)
{
	int	index;
	int	offset;

	index = n >> 3;
	offset = n & 0x7;
	return ((bitmap[index] & (1 << offset)) != 0);
}

void	bitmap_clear(uint8_t *bitmap, uint16_t n)
{
	int	index;
	int	offset;

	index = n >> 3;
	offset = n & 0x7;
	bitmap[index] &= ~(1 << offset);
}