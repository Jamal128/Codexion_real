/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_coders.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:12 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:13 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_coders(t_codexion *codex)
{
	int			i;
	int			n;
	t_coder		*coder;

	n = codex->args.number_of_coders;
	i = 0;
	while (i < n)
	{
		coder = &codex->coders[i];
		coder->id = i;
		coder->compile_count = 0;
		coder->last_compile = codex->start_time;
		coder->codex = codex;
		coder->state = WAITING;
		coder->left = &codex->dongles[i];
		coder->right = &codex->dongles[(i + 1) % n];
		if (pthread_mutex_init(&coder->mutex_coder, NULL))
			return (0);
		i++;
		codex->n_coders_init = i;
	}
	return (1);
}
