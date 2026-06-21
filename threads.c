/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:53 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:43 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_started_threads(t_codexion *codex, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(codex->coders[i].thread, NULL);
		i++;
	}
}

int	launch_threads(t_codexion *codex)
{
	int	i;
	int	n;

	n = codex->args.number_of_coders;
	i = 0;
	while (i < n)
	{
		if (pthread_create(&codex->coders[i].thread, NULL,
				coder_routine, &codex->coders[i]) != 0)
		{
			set_stopped_true(codex);
			join_started_threads(codex, i);
			return (1);
		}
		i++;
	}
	if (pthread_create(&codex->monitoring->monitoring, NULL,
			monitor_routine, codex) != 0)
	{
		set_stopped_true(codex);
		join_started_threads(codex, n);
		return (1);
	}
	return (0);
}

void	join_threads(t_codexion *codex)
{
	join_started_threads(codex, codex->args.number_of_coders);
	pthread_join(codex->monitoring->monitoring, NULL);
}
