/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_codexion.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:19 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:17 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_monitoring(t_codexion *codex)
{
	codex->monitoring = malloc(sizeof(t_monitoring));
	if (!codex->monitoring)
		return (0);
	if (pthread_mutex_init(&codex->monitoring->stop_lock, NULL))
	{
		free(codex->monitoring);
		codex->monitoring = NULL;
		return (0);
	}
	codex->monitoring->is_stopped = false;
	return (1);
}

static int	alloc_arrays(t_codexion *codex)
{
	long long	n;

	n = codex->args.number_of_coders;
	codex->coders = malloc(sizeof(t_coder) * n);
	codex->dongles = malloc(sizeof(t_dongle) * n);
	if (!codex->coders || !codex->dongles)
		return (0);
	return (1);
}

int	init_codexion(t_codexion *codex, t_args *args)
{
	codex->args = *args;
	codex->start_time = get_time_ms();
	codex->coders = NULL;
	codex->dongles = NULL;
	codex->n_dongles_init = 0;
	codex->n_coders_init = 0;
	if (pthread_mutex_init(&codex->write_mutex, NULL))
		return (0);
	if (!init_monitoring(codex))
	{
		pthread_mutex_destroy(&codex->write_mutex);
		return (0);
	}
	if (!alloc_arrays(codex) || !init_dongles(codex) || !init_coders(codex))
	{
		destroy_codexion(codex);
		return (0);
	}
	return (1);
}
