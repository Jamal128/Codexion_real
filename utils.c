/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:57 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:47 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_ms	timeval_to_ms(struct timeval time)
{
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

t_ms	get_time_ms(void)
{
	struct timeval	time;

	if (gettimeofday(&time, NULL) == -1)
	{
		if (write(2, "gettimeofday() error\n", 21) < 0)
			return (0);
	}
	return (timeval_to_ms(time));
}

int	ft_usleep(t_codexion *codex, t_ms milliseconds)
{
	t_ms	start;

	start = get_time_ms();
	while (!is_stopped(codex) && (get_time_ms() - start) < milliseconds)
		usleep(100);
	return (0);
}
