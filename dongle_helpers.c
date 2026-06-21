/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:47 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:52 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_ms	calc_priority(t_coder *coder)
{
	t_ms	base_prio;

	if (coder->codex->args.scheduler == FIFO)
		base_prio = get_time_ms();
	else
		base_prio = coder->last_compile + coder->codex->args.time_to_burnout;
	return (base_prio * 1000000 + (coder->id % 2) * 1000 + coder->id);
}

int	cooldown_passed(t_dongle *dongle, t_ms cooldown)
{
	if (dongle->last_use_time == 0)
		return (1);
	return ((get_time_ms() - dongle->last_use_time) >= cooldown);
}

int	is_queue_head(t_dongle *dongle, int coder_id)
{
	int	head_id;

	pthread_mutex_lock(&dongle->queue->heap_lock);
	head_id = get_id(dongle->queue);
	pthread_mutex_unlock(&dongle->queue->heap_lock);
	return (head_id == coder_id);
}

t_timespec	calc_timeout(t_dongle *dongle, t_codexion *codex)
{
	t_timespec		ts;
	t_ms			wait_until_ms;
	t_ms			fallback;

	if (dongle->last_use_time != 0)
		wait_until_ms = dongle->last_use_time + codex->args.dongle_cooldown;
	else
	{
		fallback = codex->args.time_to_compile + codex->args.dongle_cooldown;
		wait_until_ms = get_time_ms() + fallback + 50;
	}
	ts.tv_sec = wait_until_ms / 1000;
	ts.tv_nsec = (wait_until_ms % 1000) * 1000000L;
	return (ts);
}
