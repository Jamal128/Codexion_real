/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:26 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:24 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_codexion	codex;
	t_args		args;

	if (!checker_parse_args(argc, argv, &args))
	{
		fprintf(stderr, "Usage: codexion number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n");
		return (1);
	}
	if (!init_codexion(&codex, &args))
	{
		fprintf(stderr, "Error: init failed\n");
		return (1);
	}
	if (launch_threads(&codex) != 0)
	{
		fprintf(stderr, "Error: thread creation failed\n");
		destroy_codexion(&codex);
		return (1);
	}
	join_threads(&codex);
	destroy_codexion(&codex);
	return (0);
}
