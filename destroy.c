/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   destroy.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:39 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:45 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_monitoring(t_codexion *codex)
{
	if (!codex->monitoring)
		return ;
	pthread_mutex_destroy(&codex->monitoring->stop_lock);
	free(codex->monitoring);
}

void	destroy_codexion(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->n_dongles_init)
	{
		pthread_mutex_destroy(&codex->dongles[i].mutex);
		pthread_cond_destroy(&codex->dongles[i].cond);
		destroy_queue(codex->dongles[i].queue);
		i++;
	}
	i = 0;
	while (i < codex->n_coders_init)
	{
		pthread_mutex_destroy(&codex->coders[i].mutex_coder);
		i++;
	}
	destroy_monitoring(codex);
	pthread_mutex_destroy(&codex->write_mutex);
	free(codex->coders);
	free(codex->dongles);
}
