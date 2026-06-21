/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_init.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:44 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:35 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_queue(t_queue *queue, int capacity)
{
	queue->nodes = malloc(sizeof(t_qnode) * capacity);
	if (!queue->nodes)
		return (0);
	if (pthread_mutex_init(&queue->heap_lock, NULL))
	{
		free(queue->nodes);
		return (0);
	}
	queue->size = 0;
	queue->capacity = capacity;
	return (1);
}

void	destroy_queue(t_queue *queue)
{
	pthread_mutex_destroy(&queue->heap_lock);
	free(queue->nodes);
	free(queue);
}
