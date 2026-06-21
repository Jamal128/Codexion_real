/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_dongles.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:23 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:21 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_one_dongle(t_dongle *dongle, int n)
{
	dongle->available = true;
	dongle->last_use_time = 0;
	if (pthread_mutex_init(&dongle->mutex, NULL))
		return (0);
	if (pthread_cond_init(&dongle->cond, NULL))
	{
		pthread_mutex_destroy(&dongle->mutex);
		return (0);
	}
	dongle->queue = malloc(sizeof(t_queue));
	if (!dongle->queue || !init_queue(dongle->queue, n))
	{
		pthread_mutex_destroy(&dongle->mutex);
		pthread_cond_destroy(&dongle->cond);
		free(dongle->queue);
		return (0);
	}
	return (1);
}

int	init_dongles(t_codexion *codex)
{
	int	i;
	int	n;

	n = codex->args.number_of_coders;
	i = 0;
	while (i < n)
	{
		codex->dongles[i].id = i;
		if (!init_one_dongle(&codex->dongles[i], n))
			return (0);
		i++;
		codex->n_dongles_init = i;
	}
	return (1);
}
