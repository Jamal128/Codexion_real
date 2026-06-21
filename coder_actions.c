/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_actions.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:07 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:29 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	change_state(t_coder *coder, t_state new_state)
{
	pthread_mutex_lock(&coder->mutex_coder);
	coder->state = new_state;
	pthread_mutex_unlock(&coder->mutex_coder);
}

void	do_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->mutex_coder);
	coder->last_compile = get_time_ms();
	coder->state = COMPILING;
	pthread_mutex_unlock(&coder->mutex_coder);
	print_logs(coder->id, COMPILING, coder->codex);
	ft_usleep(coder->codex, coder->codex->args.time_to_compile);
	pthread_mutex_lock(&coder->mutex_coder);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->mutex_coder);
}

void	do_debug(t_coder *coder)
{
	change_state(coder, DEBUGGING);
	print_logs(coder->id, DEBUGGING, coder->codex);
	ft_usleep(coder->codex, coder->codex->args.time_to_debug);
}

void	do_refactor(t_coder *coder)
{
	change_state(coder, REFACTORING);
	print_logs(coder->id, REFACTORING, coder->codex);
	ft_usleep(coder->codex, coder->codex->args.time_to_refactor);
}
