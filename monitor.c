/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:31 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:27 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	coder_burned_out(t_coder *coder)
{
	t_ms	now;
	t_ms	last;

	pthread_mutex_lock(&coder->mutex_coder);
	last = coder->last_compile;
	pthread_mutex_unlock(&coder->mutex_coder);
	now = get_time_ms();
	return (now - last >= coder->codex->args.time_to_burnout);
}

void	*monitor_routine(void *arg)
{
	t_codexion	*codex;
	int			i;

	codex = (t_codexion *)arg;
	while (!is_stopped(codex))
	{
		i = 0;
		while (i < codex->args.number_of_coders)
		{
			if (coder_burned_out(&codex->coders[i]))
			{
				print_logs(codex->coders[i].id, BURNED_OUT, codex);
				set_stopped_true(codex);
				return (NULL);
			}
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
