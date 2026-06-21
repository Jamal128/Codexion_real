/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_acquire.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:44 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:48 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	dongle_ready(t_dongle *d, int coder_id, t_ms cooldown)
{
	return (is_queue_head(d, coder_id) && d->available
		&& cooldown_passed(d, cooldown));
}

static int	try_take_both(t_dongle *first, t_dongle *second, t_coder *coder)
{
	t_ms	cooldown;

	cooldown = coder->codex->args.dongle_cooldown;
	if (!dongle_ready(first, coder->id, cooldown)
		|| !dongle_ready(second, coder->id, cooldown))
		return (0);
	pop(first->queue);
	first->available = false;
	pop(second->queue);
	second->available = false;
	return (1);
}

static void	wait_turn(t_dongle *holder, t_dongle *blocker, t_coder *coder)
{
	t_timespec	ts;

	pthread_mutex_unlock(&holder->mutex);
	ts = calc_timeout(blocker, coder->codex);
	pthread_cond_timedwait(&blocker->cond, &blocker->mutex, &ts);
	pthread_mutex_unlock(&blocker->mutex);
}

int	acquire_loop(t_dongle *first, t_dongle *second, t_coder *coder)
{
	t_ms	cooldown;

	cooldown = coder->codex->args.dongle_cooldown;
	while (!is_stopped(coder->codex))
	{
		pthread_mutex_lock(&first->mutex);
		pthread_mutex_lock(&second->mutex);
		if (try_take_both(first, second, coder))
		{
			pthread_mutex_unlock(&second->mutex);
			pthread_mutex_unlock(&first->mutex);
			return (1);
		}
		if (dongle_ready(first, coder->id, cooldown))
			wait_turn(first, second, coder);
		else
			wait_turn(second, first, coder);
	}
	return (0);
}
