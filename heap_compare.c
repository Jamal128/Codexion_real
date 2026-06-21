/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_compare.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:59 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:02 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	swap_nodes(t_qnode *a, t_qnode *b)
{
	t_qnode	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	get_prio(t_qnode *a, t_qnode *b)
{
	return (a->priority > b->priority);
}

int	get_id(t_queue *queue)
{
	if (queue->size == 0)
		return (-1);
	return (queue->nodes[0].coder_id);
}
