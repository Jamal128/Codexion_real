/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop_check.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:48 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:39 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wake_all_dongles(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->args.number_of_coders)
	{
		pthread_mutex_lock(&codex->dongles[i].mutex);
		pthread_cond_broadcast(&codex->dongles[i].cond);
		pthread_mutex_unlock(&codex->dongles[i].mutex);
		i++;
	}
}

void	set_stopped_true(t_codexion *codex)
{
	pthread_mutex_lock(&codex->monitoring->stop_lock);
	codex->monitoring->is_stopped = true;
	pthread_mutex_unlock(&codex->monitoring->stop_lock);
	wake_all_dongles(codex);
}

bool	is_stopped(t_codexion *codex)
{
	bool	stopped;

	pthread_mutex_lock(&codex->monitoring->stop_lock);
	stopped = codex->monitoring->is_stopped;
	pthread_mutex_unlock(&codex->monitoring->stop_lock);
	return (stopped);
}

int	all_done(t_codexion *codex)
{
	int	i;
	int	count;

	i = 0;
	while (i < codex->args.number_of_coders)
	{
		pthread_mutex_lock(&codex->coders[i].mutex_coder);
		count = codex->coders[i].compile_count;
		pthread_mutex_unlock(&codex->coders[i].mutex_coder);
		if (count < codex->args.number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}
