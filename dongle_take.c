/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_take.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:55 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:59 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	sort_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left->id < coder->right->id)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}

static void	register_request(t_dongle *first, t_dongle *second,
		t_coder *coder, t_ms prio)
{
	pthread_mutex_lock(&first->mutex);
	insert(first->queue, coder->id, prio);
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	insert(second->queue, coder->id, prio);
	pthread_mutex_unlock(&second->mutex);
}

static void	cancel_request(t_dongle *first, t_dongle *second, int coder_id)
{
	pthread_mutex_lock(&first->mutex);
	remove_from_queue(first->queue, coder_id);
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	remove_from_queue(second->queue, coder_id);
	pthread_mutex_unlock(&second->mutex);
}

int	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	int			ok;

	sort_dongles(coder, &first, &second);
	register_request(first, second, coder, calc_priority(coder));
	ok = acquire_loop(first, second, coder);
	if (ok)
	{
		print_logs(coder->id, TAKING_DONGLE, coder->codex);
		print_logs(coder->id, TAKING_DONGLE, coder->codex);
	}
	else
		cancel_request(first, second, coder->id);
	return (ok);
}
