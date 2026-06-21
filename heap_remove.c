/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_remove.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:08 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:10 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	heapify_down(t_queue *queue, int i)
{
	int	left;
	int	right;
	int	best;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		best = i;
		if (left < queue->size && get_prio(&queue->nodes[left],
				&queue->nodes[best]))
			best = left;
		if (right < queue->size && get_prio(&queue->nodes[right],
				&queue->nodes[best]))
			best = right;
		if (best == i)
			break ;
		swap_nodes(&queue->nodes[i], &queue->nodes[best]);
		i = best;
	}
}

void	pop(t_queue *queue)
{
	pthread_mutex_lock(&queue->heap_lock);
	if (queue->size == 0)
	{
		pthread_mutex_unlock(&queue->heap_lock);
		return ;
	}
	queue->nodes[0] = queue->nodes[queue->size - 1];
	queue->size--;
	if (queue->size > 0)
		heapify_down(queue, 0);
	pthread_mutex_unlock(&queue->heap_lock);
}

void	remove_from_queue(t_queue *queue, int coder_id)
{
	int	i;

	pthread_mutex_lock(&queue->heap_lock);
	i = 0;
	while (i < queue->size)
	{
		if (queue->nodes[i].coder_id == coder_id)
		{
			queue->nodes[i] = queue->nodes[queue->size - 1];
			queue->size--;
			if (i < queue->size)
			{
				heapify_up(queue, i);
				heapify_down(queue, i);
			}
			break ;
		}
		i++;
	}
	pthread_mutex_unlock(&queue->heap_lock);
}
