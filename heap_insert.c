/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_insert.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:05 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:06 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heapify_up(t_queue *queue, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!get_prio(&queue->nodes[parent], &queue->nodes[i]))
			break ;
		swap_nodes(&queue->nodes[i], &queue->nodes[parent]);
		i = parent;
	}
}

void	insert(t_queue *queue, int id, t_ms priority)
{
	t_qnode	*node;

	pthread_mutex_lock(&queue->heap_lock);
	if (queue->size >= queue->capacity)
	{
		pthread_mutex_unlock(&queue->heap_lock);
		return ;
	}
	node = &queue->nodes[queue->size];
	node->coder_id = id;
	node->priority = priority;
	queue->size++;
	heapify_up(queue, queue->size - 1);
	pthread_mutex_unlock(&queue->heap_lock);
}
