/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_logs.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:39 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:32 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_logs(int index, t_state state, t_codexion *codex)
{
	t_ms	time;

	pthread_mutex_lock(&codex->write_mutex);
	time = get_time_ms() - codex->start_time;
	if (state == TAKING_DONGLE)
		printf(LOG_TAKE_DONGLE, time, index + 1);
	else if (state == COMPILING)
		printf(LOG_COMPILING, time, index + 1);
	else if (state == DEBUGGING)
		printf(LOG_DEBUGGING, time, index + 1);
	else if (state == REFACTORING)
		printf(LOG_REFACTOR, time, index + 1);
	else if (state == BURNED_OUT)
		printf(LOG_BURNS_OUT, time, index + 1);
	pthread_mutex_unlock(&codex->write_mutex);
}
