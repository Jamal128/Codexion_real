/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_release.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:51 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:55 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	release_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	sort_dongles(coder, &first, &second);
	pthread_mutex_lock(&first->mutex);
	first->available = true;
	first->last_use_time = get_time_ms();
	pthread_cond_broadcast(&first->cond);
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	second->available = true;
	second->last_use_time = get_time_ms();
	pthread_cond_broadcast(&second->cond);
	pthread_mutex_unlock(&second->mutex);
}
