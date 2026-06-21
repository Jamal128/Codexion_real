/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:18 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:34 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	run_cycle(t_coder *coder)
{
	change_state(coder, WAITING);
	if (!take_dongles(coder))
		return (0);
	do_compile(coder);
	release_dongles(coder);
	if (all_done(coder->codex))
	{
		set_stopped_true(coder->codex);
		return (0);
	}
	if (is_stopped(coder->codex))
		return (0);
	do_debug(coder);
	if (is_stopped(coder->codex))
		return (0);
	do_refactor(coder);
	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->codex->args.number_of_coders == 1)
	{
		ft_usleep(coder->codex, coder->codex->args.time_to_burnout);
		change_state(coder, FINISHED);
		return (NULL);
	}
	while (!is_stopped(coder->codex) && run_cycle(coder))
		;
	change_state(coder, FINISHED);
	return (NULL);
}
